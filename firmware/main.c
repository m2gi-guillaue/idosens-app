
/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Idosens sensor application
 *
 * @}
 */

#include "board.h"
#include "periph_conf.h"
#include "clk.h"
#include "timex.h"
#include "ztimer.h"
#include "at30tse75x.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "msg.h"
#include "thread.h"
#include "fmt.h"

#include "net/loramac.h"
#include "semtech_loramac.h"

#include "sx127x.h"
#include "sx127x_netdev.h"
#include "sx127x_params.h"

#define PERIOD_S            (60U)

#define SENDER_PRIO         (THREAD_PRIORITY_MAIN - 1)
static kernel_pid_t sender_pid;
static char sender_stack[THREAD_STACKSIZE_MAIN / 2];

static semtech_loramac_t loramac;

static sx127x_t sx127x;

static ztimer_t timer;

static at30tse75x_t temp_driver;

static uint8_t deveui[LORAMAC_DEVEUI_LEN];
static uint8_t appeui[LORAMAC_APPEUI_LEN];
static uint8_t appkey[LORAMAC_APPKEY_LEN];



static void _alarm_cb(void *arg)
{
    (void) arg;
    msg_t msg;
    msg_send(&msg, sender_pid);
}

static void _prepare_next_alarm(void)
{
    timer.callback = _alarm_cb;
    ztimer_set(ZTIMER_MSEC, &timer, PERIOD_S * MS_PER_SEC);
}

static void _send_message(void)
{
    float temp = 0.0f;
    puts("envoi du message sur LORA");
    if (at30tse75x_get_temperature(&temp_driver, &temp) == 0){
        /* Try to send the message */
        uint8_t ret = semtech_loramac_send(&loramac,
                                            (uint8_t *) &temp, sizeof(temp));
        if (ret != SEMTECH_LORAMAC_TX_DONE)  {
            printf("Cannot send message");
            return;
        }
    }
    
}

static void *sender(void *arg)
{
    (void)arg;

    msg_t msg;
    msg_t msg_queue[8];
    msg_init_queue(msg_queue, 8);

    while (1) {
        msg_receive(&msg);

        /* Trigger the message send */
        _send_message();

        /* Schedule the next wake-up alarm */
        _prepare_next_alarm();
    }

    /* this should never be reached */
    return NULL;
}

int main(void)
{

    puts("Starting temperature driver");
    while (at30tse75x_init(&temp_driver, PORT_A, AT30TSE75X_TEMP_ADDR) != 0) {
        puts("error temperature driver sensor");
        delay();
    };
    puts("Starting init temperature driver succeeded");

    /* Convert identifiers and application key */
    fmt_hex_bytes(deveui, CONFIG_LORAMAC_DEV_EUI_DEFAULT);
    fmt_hex_bytes(appeui, CONFIG_LORAMAC_APP_EUI_DEFAULT);
    fmt_hex_bytes(appkey, CONFIG_LORAMAC_APP_KEY_DEFAULT);

    /* Initialize the radio driver */
    sx127x_setup(&sx127x, &sx127x_params[0], 0);
    loramac.netdev = &sx127x.netdev;
    loramac.netdev->driver = &sx127x_driver;

    /* Initialize the loramac stack */
    semtech_loramac_init(&loramac);
    semtech_loramac_set_deveui(&loramac, deveui);
    semtech_loramac_set_appeui(&loramac, appeui);
    semtech_loramac_set_appkey(&loramac, appkey);

    /* Use a fast datarate, e.g. BW125/SF7 in EU868 */
    semtech_loramac_set_dr(&loramac, LORAMAC_DR_5 );

    /* Start the Over-The-Air Activation (OTAA) procedure to retrieve the
     * generated device address and to get the network and application session
     * keys.
     */
    puts("Starting join procedure");
    while (semtech_loramac_join(&loramac, LORAMAC_JOIN_OTAA) != SEMTECH_LORAMAC_JOIN_SUCCEEDED) {
        puts("Join procedure failed");
        ztimer_sleep(ZTIMER_USEC, 20 * US_PER_SEC);
    }
    puts("Join procedure succeeded");

        /* start the sender thread */
    sender_pid = thread_create(sender_stack, sizeof(sender_stack),
                               SENDER_PRIO, 0, sender, NULL, "sender");

    /* trigger the first send */
    msg_t msg;
    msg_send(&msg, sender_pid);
    return 0;
    
}
