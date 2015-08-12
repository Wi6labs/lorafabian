/*--------------------------------------------------------------------------

Copyright (c) <2015>, <Wi6labs>, <Telecom Bretagne>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Telecom Bretagne, wi6labs nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL TELECOM BRETAGNE OR WI6LABS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Description:
LoRaFabian Beacon Answer code. LoRa RX setq sx1272 in receive mode on the
channel configured in sx1272_contiki_radio.c

-----------------------------------------------------------------------------*/                                                             

#include "contiki.h"
#include "leds-arch.h"
#include "stm32f10x.h"
#include <dev/leds.h>
#include <stdio.h>
#include "layer802154_radio_lora.h"
#include "arduino_spi.h"
#include "debug_on_arduino.h"
#include "er-coap.h"
#include "er-coap-constants.h"
#include "frame_manager.h"
#include "frame802154_lora.h"
#include "cfs/cfs.h"
#include <string.h>

static struct etimer rx_timer;
static struct etimer timer_payload_beacon;

//The response to the beacon
char coap_payload_beacon[150];

static int is_associated = 0;
static int is_beacon_receive = 0;

void frame_manager_init()
{
  process_start(&lorafab_bcn_process, NULL);
}

/**
 * \brief: update the hostname with /HOSTNAME file
 */
void updateHOSTNAME()
{
  char dns[150];//The content of the file
  int fd;
  //Read in /HOSTNAME_LORA
  fd = cfs_open("/HOSTNAME_LORA", CFS_READ);
  if(fd >= 0) {
    //Read 500 char
    cfs_read(fd, dns, sizeof(dns));
    cfs_close(fd);
    //Get the real hostname
    int size = 0;
    //Because the space significate the end of the hostname
    while(dns[size] != '\0')
      ++size;
    //final = the real url
    char final[size];
    int i;
    for(i = 0; i != sizeof(final) +1; ++i)
      final[i] = dns[i];
    strcpy(coap_payload_beacon, "{\"n\":\"");
    strcat(coap_payload_beacon, final);
    strcat(coap_payload_beacon, "\"}");
  }
  else {
    printf("ERREUR LORS DE LA LECTURE\n\r");
    strcpy(coap_payload_beacon, "{\"n\":\"default.test\"}");
    return;
  }
  printf("HOSTNAME : %s\n\r", coap_payload_beacon);
}

/**
 * \brief: Send the coap_payload_beacon to layer802154
 */
void
coap_beacon_send_response() {
  updateHOSTNAME();
  uint8_t tx_buffer[512];

  size_t coap_packet_size;
  static coap_packet_t coap_request[1];      //This way the packet can be treated as pointer as usual.

  unsigned short random_a = random_rand();
  char MAC[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  getMac(MAC);
  unsigned short random_b = random_a ^ MAC[sizeof(MAC)-1];
  coap_init_message(coap_request, COAP_TYPE_NON, COAP_POST, (coap_get_mid()+random_b)%65535);
  int sizeMSG = strlen(coap_payload_beacon);
  coap_set_payload(coap_request, (uint8_t *)coap_payload_beacon, sizeMSG);

  coap_packet_size = coap_serialize_message(coap_request, (void *)(tx_buffer ));
  int tx_buffer_index = coap_packet_size;
  printf("We are sending the response to the coap beacon\n\r");

  //We must write a destination address on 8 bytes
  uint8_t destAddr[] = {0xfa,0x01};
  layer802154_send(tx_buffer, tx_buffer_index, destAddr, SIGNALISATION_ON, DST_SHORT_FLAG);

  is_associated = 1;
}

/**
 * \brief: Check if the payload is a beacon payload
 * \param: rx_msg, the payload
 * \param: size, the payload length
 */
int respond_if_coap_beacon(u8 rx_msg[], int size) {
  printf("\n\r\tPayload: ");
  int iaux;
  for(iaux = 0 ; iaux<size ; iaux++)
    printf("%02x", rx_msg[iaux]);

   //static declaration reduces stack peaks and program code size
  static coap_packet_t coap_message[1]; //this way the packet can be treated as pointer as usual
  erbium_status_code = NO_ERROR;

  erbium_status_code =
  coap_parse_message(coap_message, (void *)(rx_msg), size);
  printf("\n\r\tPayload is CoAP Beacon?: ");
  if(erbium_status_code == NO_ERROR) {
    int check_coap_beacon = (coap_message->type == COAP_TYPE_NON) && ((coap_message->code == COAP_POST)
                            && (coap_message->payload_len == 0));
    if(check_coap_beacon){
      printf("\n\r\tThis is the LoRA CoAP Beacon\n\r");
      return 1;
    } else
        printf("Not LoRa Beacon. Other CoAP Message.\n\r");
  } else
      printf("Not LoRa Beacon. CoAP parse ERROR\n\r");
  printf("\n\r");
  return 0;
}

/*---------------------------------------------------------------------------*/
PROCESS(lorafab_bcn_process, "LoRaFabian Beacon process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(lorafab_bcn_process, ev, data) {
  PROCESS_BEGIN();

  int pending = 0;
  int i;
  int size;

  leds_init();
  leds_toggle(LEDS_ALL);
  arduino_spi_init();
  layer802154_init();
  //Start infinite RX
  layer802154_on();

  etimer_set(&rx_timer, 5*CLOCK_SECOND);

  while(1) {
    PROCESS_WAIT_EVENT();

    if(is_beacon_receive && !is_associated && etimer_expired(&timer_payload_beacon))
    {
      etimer_stop(&timer_payload_beacon);
      coap_beacon_send_response();
    }

    if(etimer_expired(&rx_timer)) {
      leds_toggle(LEDS_ALL);

      pending = layer802154_pending_packet();
      printf("pending_packet: %d\n\r", pending);

      if(pending) {
        frame802154_lora_t frame = layer802154_read();
        size = frame.payload_len;

        if(frame.header_len == -1)
          printf("Error: buffer is too small for headers");
        else {
          //For the arduino
          int packetSize = size + frame.header_len;
          //Verify the destination of a message
          bool br_msg = is_broadcast_addr(&frame);
          bool my_mac = is_my_mac(&frame);
          if(br_msg) {
            printf("Broadcast message");
            if(!is_signaling(&frame) || debug_on_arduino)
              set_arduino_read_buf(frame.packet, packetSize);
          }
          else if(my_mac) {
            printf("Message is for me");
            set_arduino_read_buf(frame.payload, frame.payload_len);
           }
          else {
            printf("Message is not for me");
            if(debug_on_arduino)
              set_arduino_read_buf(frame.packet, packetSize);
          }

      //To avoid collision
          if(respond_if_coap_beacon(frame.payload, size) && !is_associated)
          {
            is_beacon_receive = 1;
            int random_timer = (random_rand()%30);
            etimer_set(&timer_payload_beacon, random_timer*CLOCK_SECOND);
          }
        }
      }
      etimer_reset(&rx_timer);
    }
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
