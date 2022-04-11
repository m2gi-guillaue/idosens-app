# name of your application
APPLICATION = idosens-app

# If no BOARD is found in the environment, use this default:
BOARD ?= native

DRIVER ?= sx1276

LORA_REGION ?= EU868

USEPKG += semtech-loramac

USEMODULE += $(DRIVER)
USEMODULE += fmt
FEATURES_OPTIONAL += periph_rtc

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

DEVEUI ?= 5059b3bb4101e86b
APPEUI ?= 0000000000000000
APPKEY ?= CD64DAA67DCB5AA4DC9E8148FAA3EFFA

# Pass these enviroment variables to docker
DOCKER_ENV_VARS += DEVEUI
DOCKER_ENV_VARS += APPEUI
DOCKER_ENV_VARS += APPKEY

# we want to use SAUL:
USEMODULE += at30tse75x
# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include

ifndef CONFIG_KCONFIG_USEMODULE_LORAWAN
  # OTAA compile time configuration keys
  CFLAGS += -DCONFIG_LORAMAC_APP_KEY_DEFAULT=\"$(APPKEY)\"
  CFLAGS += -DCONFIG_LORAMAC_APP_EUI_DEFAULT=\"$(APPEUI)\"
  CFLAGS += -DCONFIG_LORAMAC_DEV_EUI_DEFAULT=\"$(DEVEUI)\"
endif
