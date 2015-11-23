#include "frame802154_lora.h"

const uint8_t HARDCODED_MAC[] = {0xfa,0xb0,0x00,0x00,0x00,0x00,0x00,0x03};
const uint8_t GATEWAY_ADDR[] = {0xfa,0x01};

void getMac(uint8_t *buffer)
{
  memcpy(buffer, &HARDCODED_MAC, sizeof(HARDCODED_MAC));
}

//Verify if addr is a broadcast address
int
is_broadcast_addr(frame802154_lora_t *frame)
{
  if(frame->fcf._10_11_dst_addr_mode == ADDRESS_0b_LENGTH)
    return 1;
  int i =0;
  int dstAddSize = frame->fcf._10_11_dst_addr_mode == ADDRESS_16b_LENGTH ? 1 : 7;
  for (i;i<=dstAddSize;i++) {
        if(frame->dest_addr[i] != 0xff)
           return 0;
   }
   return 1;
}

//Verify is add correspond to the hardcoded_mac
int
is_my_mac(frame802154_lora_t *frame)
{
  int i = 0;
  int dstAddSize = frame->fcf._10_11_dst_addr_mode == ADDRESS_16b_LENGTH ? 1 : 7;
  for (i;i<=dstAddSize;i++) {
    if(frame->dest_addr[i] != HARDCODED_MAC[i])
      return 0;
  }
  return 1;
}

//Verify if the signalling bit is set in the src mac address of the received packet
int
is_signaling(frame802154_lora_t *frame)
{
  int i = frame->fcf._14_15_src_addr_mode == ADDRESS_16b_LENGTH ? 1 : 7;
  // return 1 if MSB of last byte in src mac is set, else 0
  return ((frame->src_addr[i] & (0b10000000))>>7);
}

/**
 *   \brief Parses an input frame.  Scans the input frame to find each
 *   section, and stores the information of each section in a
 *   frame802154_lora_t structure.
 *
 *   \param data The input data from the radio chip.
 *   \param len The size of the input data
 *   \param pf The frame802154_lora_t struct to store the parsed frame information.
 */
int
frame802154_lora_parse(uint8_t *data, int len, frame802154_lora_t *pf)
{
  uint8_t *p;
  frame802154_lora_fcf_t fcf;
  int c;

  if(len < MIN_FRAME_LENGTH)
    return -1;

  pf->packet = data;
  p = data;

  //decode the FCF
  fcf._0_2_frame_type = (p[0] & (0b00100000)) >> 5;
  fcf._3_security_enabled = (p[0] & (0b00010000)) >> 4;
  fcf._4_frame_pending = (p[0] & (0b00001000)) >> 3;
  fcf._5_ack_request = (p[0] & (0b00000100)) >> 2;
  fcf._6_pan_id_compression = (p[0] & (0b00000010)) >> 1;

  fcf._10_11_dst_addr_mode = (p[1] & (0b00110000)) >> 4;
  fcf._12_13_frame_ver = (p[1] & (0b00001100)) >> 2;
  fcf._14_15_src_addr_mode = (p[1] & (0b00000011));

  //copy fcf and seqNum
  memcpy(&pf->fcf, &fcf, sizeof(frame802154_lora_fcf_t));
  pf->seq = p[2];
  p += 3;

  if(fcf._10_11_dst_addr_mode == fcf._14_15_src_addr_mode)
    return -1;
  
  int dstAddSize = (fcf._10_11_dst_addr_mode == ADDRESS_16b_LENGTH)? 2 : 
                   (fcf._10_11_dst_addr_mode == ADDRESS_0b_LENGTH) ? 0 : 8;
  int i = 0;
  for(i; i < dstAddSize; ++i)
    pf->dest_addr[i] = p[i];
  
  int srcAddSize = (fcf._14_15_src_addr_mode == ADDRESS_16b_LENGTH)? 2:8;
  int j = 0;
  for(j; j < srcAddSize; ++j)
    pf->src_addr[j] = p[j+i];
  p+= dstAddSize + srcAddSize;
    
  //header length
  c = p - data;
  //payload length
  pf->payload_len = (len - c);
  //payload
  pf->payload = p;

  /* return header length if successful */
  return c > len ? -1 : c;
}



static void
field_len(frame802154_lora_t *p, field_length_t *flen)
{
  /* init flen to zeros */
  memset(flen, 0, sizeof(field_length_t));

  /* determine address lengths */
  flen->dest_addr_len = (p->fcf._10_11_dst_addr_mode & 3) == ADDRESS_16b_LENGTH ? 2 : 8;
  flen->src_addr_len = (p->fcf._14_15_src_addr_mode & 3) == ADDRESS_16b_LENGTH ? 2 : 8;
}

/*----------------------------------------------------------------------------*/
/**
 *   \brief Creates a frame for transmission over the air.  This function is
 *   meant to be called by a higher level function, that interfaces to a MAC.
 *
 *   \param p Pointer to frame802154_lora_t struct, which specifies the
 *   frame to send.
 *
 *   \param buf Pointer to the buffer to use for the frame.
 *
 *   \param buf_len The length of the buffer to use for the frame.
 *
 *   \return The length of the frame header or 0 if there was
 *   insufficient space in the buffer for the frame headers.
*/
int
frame802154_lora_create(frame802154_lora_t *p, uint8_t *buf, int buf_len)
{
  int c;
  field_length_t flen;
  uint8_t *tx_frame_buffer;
  uint8_t pos;

  field_len(p, &flen);

  if(3 + flen.dest_addr_len +  flen.src_addr_len > buf_len) {
    /* Too little space for headers. */
    return -1;
  }

  /* OK, now we have field lengths.  Time to actually construct */
  /* the outgoing frame, and store it in tx_frame_buffer */
  tx_frame_buffer = buf;
  tx_frame_buffer[0] = ((p->fcf._0_2_frame_type & 7) << 5) |
    ((p->fcf._3_security_enabled & 1) << 4) |
    ((p->fcf._4_frame_pending & 1) << 3) |
    ((p->fcf._5_ack_request & 1) << 2) |
    ((p->fcf._6_pan_id_compression & 1) << 1);
  tx_frame_buffer[1] = ((p->fcf._10_11_dst_addr_mode & 3) << 4) |
    ((p->fcf._12_13_frame_ver & 3) << 2) |
    ((p->fcf._14_15_src_addr_mode & 3));

  /* sequence number */
  tx_frame_buffer[2] = p->seq;
  pos = 3;

  /* Destination address */
  for(c = 0; c < flen.dest_addr_len; c++) {
    tx_frame_buffer[pos++] = p->dest_addr[c];
  }

  /* Source address */
  for(c = 0; c < flen.src_addr_len; c++) {
    tx_frame_buffer[pos++] = p->src_addr[c];
  }

  return (int)pos;
}

int
frame802154_lora_hdrlen(frame802154_lora_t *p)
{
  field_length_t flen;
  field_len(p, &flen);
  return 3 + flen.dest_addr_len + flen.src_addr_len;
}


/*---------------------------------------------------------------------------*/
int
add_802_15_4_header(void *buf, uint8_t* destAddr, int signalisation, int dstShortSrcLongFlag)
{
  int header_size = 13;
  //Init the 802.15.4 frame
  //header 0x22
  frame802154_lora_t frame;
  frame.fcf._0_2_frame_type = 0x01;
  frame.fcf._3_security_enabled = 0x00;
  frame.fcf._4_frame_pending = 0x00;
  frame.fcf._5_ack_request = 0x00;
  frame.fcf._6_pan_id_compression = 0x01;
  //Address Mode
  frame.fcf._10_11_dst_addr_mode = dstShortSrcLongFlag? 0x02:0x03;
  frame.fcf._12_13_frame_ver = 0x01;
  frame.fcf._14_15_src_addr_mode = dstShortSrcLongFlag? 0x03:0x02;
  frame.seq = 0x01;
  //Destination
  uint8_t destAddrTemp[8];  
  memcpy(destAddrTemp, destAddr, 8);
  if(signalisation) {
    int posSignalisation = dstShortSrcLongFlag? 1:7;
    destAddrTemp[posSignalisation] = destAddrTemp[posSignalisation] | (0x80);
  }
  memcpy(frame.dest_addr, destAddrTemp, sizeof(destAddrTemp));
  //Src
  memcpy(frame.src_addr, HARDCODED_MAC, sizeof(HARDCODED_MAC));
  
  //Too little space for headers
  int err = frame802154_lora_create(&frame, buf, header_size);
  if(err == -1)
    return err;
  return header_size;
}
