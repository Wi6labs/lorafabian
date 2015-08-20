#include "layer802154_radio_lora.h"

#include "contiki.h"
#include "leds-arch.h"
#include "stm32f10x.h"
#include <dev/leds.h>
#include <stdio.h>
#include "arduino_spi.h"
#include "er-coap.h"
#include "er-coap-constants.h"

#include "frame802154_lora.h"
#include "sx1272_contiki_radio.h"
#include "packetbuf.h"
#include "contiki-conf.h"

int
layer802154_init(void)
{
  return lora_radio_driver.init();
}

int
layer802154_on(void)
{
  return lora_radio_driver.on();
}

int
layer802154_off(void)
{
  return lora_radio_driver.off();
}

/**
 * \brief: read and parse the 802154 packet
 * \return: a frame which contains the parsed packet
 */
frame802154_lora_t
layer802154_read()
{
  packetbuf_clear();
  uint8_t* datapb = packetbuf_dataptr();
  frame802154_lora_t frame;
  packetbuf_set_datalen(PACKETBUF_CONF_SIZE); //Because we don't know the size of the packet
  int size = lora_radio_driver.read(datapb, packetbuf_totlen());
  packetbuf_set_datalen(size); //Adjust the size
  int headerSize = frame802154_lora_parse(datapb, size, &frame);
  frame.payload_len = size-headerSize;
  frame.header_len = headerSize;
  return frame;
}

int
layer802154_channel_clear(void)
{
  return lora_radio_driver.channel_clear();
}

/**
 * \brief: create a 802154 packet and send to the radio layer
 * \param: destAddr ,the destination MAC address on 8 bytes
 * \param: signalisation, the signalisation flag
 * \param: dstShortSrcLongFlag, the mode. 1=short address mode for the destination and long for the source
 *                                        0=long address mode for the destination and short for the source
 * \return: the size of the header
 */
int
layer802154_send(const void *payload, unsigned short payload_len, uint8_t* destAddr, int signalisation, int dstShortSrcLongFlag)
{
  packetbuf_clear();
  uint8_t copieDstAdd[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  int size = dstShortSrcLongFlag ? 2 : 8;
  memcpy(copieDstAdd, destAddr, size);
  int headerSize = add_802_15_4_header(packetbuf_dataptr(), copieDstAdd, signalisation, dstShortSrcLongFlag);
  int packetSize = headerSize + payload_len;
  packetbuf_set_datalen(packetSize);
  if(headerSize > -1 && headerSize + payload_len < 513) {
    int i;
    memcpy(packetbuf_dataptr() + headerSize, payload, payload_len);
    printf("Response: ");
    for(i = 0; i < packetSize; ++i)
      printf("%02x", ((uint8_t*)packetbuf_dataptr())[i]);
    return lora_radio_driver.send(packetbuf_dataptr(), packetbuf_totlen());
  }
  return headerSize;
}

int
layer802154_receiving_packet(void)
{
  return lora_radio_driver.receiving_packet();
}

int
layer802154_pending_packet(void)
{
  return lora_radio_driver.pending_packet();
}
