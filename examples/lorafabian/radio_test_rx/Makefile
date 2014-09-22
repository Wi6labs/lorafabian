CONTIKI_PROJECT = radio_test_rx

all: obj_$(TARGET) $(CONTIKI_PROJECT)

obj_$(TARGET):
	-mkdir obj_$(TARGET)


USE_UART1_TRACE = 1
USE_USBCDC_TRACE = 0
USE_LORA_1272_RADIO = 1

#UIP_CONF_IPV6=1
#APPS = serial-shell
CONTIKI = ../../..
include $(CONTIKI)/Makefile.include
