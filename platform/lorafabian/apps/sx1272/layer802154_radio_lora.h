/**
 * \Description: 802154 interface for LoRaFabian
 **/
#ifndef __LAYER802154_LORA_H__
#define __LAYER802154_LORA_H__

#include "frame802154_lora.h"

#define SIGNALISATION_ON 1  //Signalisation flag for the header
#define SIGNALISATION_OFF 0
#define DST_SHORT_FLAG 1    //Short mode address for destination address in the header
#define DST_LONG_FLAG 0

//Prototypes
int layer802154_init(void);
int layer802154_on(void);
int layer802154_off(void);
int layer802154_channel_clear(void);
frame802154_lora_t layer802154_read();
int layer802154_send(const void *payload, unsigned short payload_len, uint8_t* destAddr, int signalisation, int dstShortSrcLongFlag);
int layer802154_receiving_packet(void);
int layer802154_pending_packet(void);

#endif //__LAYER802154_LORA_H__
