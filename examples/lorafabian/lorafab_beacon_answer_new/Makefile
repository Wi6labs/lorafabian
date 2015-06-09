CONTIKI_PROJECT = lorafab_beacon_answer_new

all: obj_$(TARGET) $(CONTIKI_PROJECT)

obj_$(TARGET):
	-mkdir obj_$(TARGET)


USE_UART1_TRACE = 1
USE_USBCDC_TRACE = 0
USE_LORA_1272_RADIO = 1

# If this is not used after TX we will not listen to beacons (RX will be off):
USE_LORAFAB_RESTART_RX_AFTER_TX = 1


#UIP_CONF_IPV6=1
#APPS = serial-shell
APPS += er-coap
APPS += rest-engine

CONTIKI = ../../..
include $(CONTIKI)/Makefile.include
