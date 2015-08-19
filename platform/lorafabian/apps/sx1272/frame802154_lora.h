#ifndef FRAME_802154_LORA_H
#define FRAME_802154_LORA_H

#include "contiki-conf.h"
#include "net/linkaddr.h"

#include "sys/cc.h"
#include <string.h>
#include <stdint.h>

#define MIN_FRAME_LENGTH 13  //For header
#define ADDRESS_0b_LENGTH 0
#define ADDRESS_16b_LENGTH 2
#define ADDRESS_64b_LENGTH 3

/**
 *  \brief: Structure that contains the Frame Control Field
 */
typedef struct {
  uint8_t _0_2_frame_type;       //3 bits
  uint8_t _3_security_enabled;   //1 bit
  uint8_t _4_frame_pending;      //1 bit
  uint8_t _5_ack_request;        //1 bit
  uint8_t _6_pan_id_compression; //1 bit //HERE WE DO NOT RESPECT THE STANDARD! We NEVER send PANID!
  //3 bits reserved
  uint8_t _10_11_dst_addr_mode;  //2 bits
  uint8_t _12_13_frame_ver;      //2 bits
  uint8_t _14_15_src_addr_mode;  //2 bits
} frame802154_lora_fcf_t;

/**
 * \brief: Structure that contains the 802.15.4 frame
 */
typedef struct {
  frame802154_lora_fcf_t fcf; //Frame control field
  uint8_t seq;                //Sequence number
  uint8_t dest_addr[8];       //Destination address
  uint8_t src_addr[8];        //Source address
  uint8_t *payload;           //Pointer to 802.15.4 frame payload
  uint8_t *packet;            //Pointer to all the packet
  int payload_len;            //Length of payload field
  int header_len;             //Length of header (-1 if an error occurs)
} frame802154_lora_t;

/**
 *  \brief: Structure that contains the lengths of the various addressing and security fields
 *  in the 802.15.4 header.  This structure is used in \ref frame802154_lora_create()
 */
typedef struct {
  uint8_t dest_addr_len;   //Length (in bytes) of destination address field */
  uint8_t src_addr_len;    //Length (in bytes) of source address field */
} field_length_t;

//Prototypes
int is_broadcast_addr(frame802154_lora_t *frame);
int is_my_mac(frame802154_lora_t *frame);
int is_signaling(frame802154_lora_t *frame);
static void field_len(frame802154_lora_t *p, field_length_t *flen);
int frame802154_lora_create(frame802154_lora_t *p, uint8_t *buf, int buf_len);
int frame802154_lora_parse(uint8_t *data, int len, frame802154_lora_t *pf);
int frame802154_lora_hdrlen(frame802154_lora_t *p);
int add_802_15_4_header(void *buf, uint8_t* destAddr, int signalisation, int dstShortSrcLongFlag);
void getMac(uint8_t *buffer);

extern const uint8_t HARDCODED_MAC[];
extern const uint8_t GATEWAY_ADDR[];

#endif
