
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
#include "tsl2561.h"
#include "tsl2561_params.h"
#include <stdio.h>


static void delay(void)
{
    if (IS_USED(MODULE_ZTIMER)) {
        ztimer_sleep(ZTIMER_USEC, 1 * US_PER_SEC);
    }
    else {
        /*
         * As fallback for freshly ported boards with no timer drivers written
         * yet, we just use the CPU to delay execution and assume that roughly
         * 20 CPU cycles are spend per loop iteration.
         *
         * Note that the volatile qualifier disables compiler optimizations for
         * all accesses to the counter variable. Without volatile, modern
         * compilers would detect that the loop is only wasting CPU cycles and
         * optimize it out - but here the wasting of CPU cycles is desired.
         */
        uint32_t loops = coreclk() / 20;
        for (volatile uint32_t i = 0; i < loops; i++) { }
    }
}

int main(void)
{
    at30tse75x_t temp_driver;
    float temp = 0.0f;

    /* tsl2561_t light_driver;
    tsl2561_params_t params = TSL2561_PARAMS;
    uint16_t light;
    char light_s[50];
    int err;

    while ((err = tsl2561_init(&light_driver, &params)) != 0){
        if (err == -1) {
            puts("I2C not available");
        } else {
            puts("not TSL2561");
        }
        delay();
    } */

    while (at30tse75x_init(&temp_driver, PORT_A, AT30TSE75X_TEMP_ADDR) != 0) {
        puts("error init driver sensor");
        delay();
    };

    while(1) {
        delay();
        if (at30tse75x_get_temperature(&temp_driver, &temp) == 0){
            printf("Temperature: %i.%03u °C\n",
                    (int)temp,
                    (unsigned)((temp - (int)temp) * 1000));
        } else {
            puts("error on get temperature");
        }
        /* light = tsl2561_read_illuminance(&light_driver);
        sprintf(light_s, "%d", light);
        puts(light_s); */
    }
    
}
