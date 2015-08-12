#include "LoraShield.h"

#include <SPI.h>

/**
 * Constructor
 */
LoraShield::LoraShield() 
{ }

/**
 * Init SPI
 */
void LoraShield::init()
{
  pinMode(SS_PIN, OUTPUT); // we use this for SS pin
  digitalWrite(SS_PIN, HIGH); 
  
  // start the SPI library:
  SPI.begin();  // wake up the SPI bus.
  #if defined(DUEBOARD) 
    SPI.setDataMode(_ss_pin, SPI_MODE0);
    SPI.setClockDivider(_ss_pin, SPI_CLK_DIVIDER);
  #else
    SPI.setDataMode(SPI_MODE0);
    SPI.setClockDivider(SPI_CLK_DIVIDER);
  #endif

  SPI.setBitOrder(MSBFIRST);   // By Default MSBFIRST,  we make this fact explicit.

  _debug = false;
  delay(100);
}


/**
 * \description: send the name of the object to the shield
 * \param: name - the name
 */
void LoraShield::begin(String name)
{
  digitalWrite(SS_PIN, LOW);

  // The byte is the status of the last command
  int previous_cmd_status = SPI.transfer(ARDUINO_CMD_HOSTNAME); 
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
  
  // Data size to be sent
  // MSB
  int shield_status = SPI.transfer(name.length() >> 8);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);

  shield_status = SPI.transfer(name.length());
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);

  //Send:  payload as bytes to send
  for (int i = 0; i < name.length() ; i++)
  {
    shield_status = SPI.transfer(name[i]);
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
  }

  digitalWrite(SS_PIN,HIGH); 
  delay(WAIT_TIME_BETWEEN_SPI_MSG*2);
}

/**
 * \description: Get the length of available data
 * \return: the length of available data
 */
int LoraShield::dataAvailable()
{
  digitalWrite(SS_PIN, LOW);

  // data available cmd
  int previous_cmd_status = SPI.transfer(ARDUINO_CMD_AVAILABLE);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
  int shield_status = SPI.transfer(ARDUINO_CMD_AVAILABLE);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);  
  int available_msb = SPI.transfer(ARDUINO_CMD_AVAILABLE);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI); 
  int available_lsb = SPI.transfer(ARDUINO_CMD_AVAILABLE);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);

  // take the SS pin high to de-select the chip:
  digitalWrite(SS_PIN,HIGH); 
  delayMicroseconds(WAIT_TIME_BETWEEN_SPI_MSG);

  return (available_msb << 8) + available_lsb;
}

/**
 * \description: Read a packet, check the CRC and respond if it is a valid COAP request (see endpoints.h)
 * \param: verbose - if we want to print some informations
 * \return: the payload of the 802.15.4 packet
 */
String LoraShield::read(bool verbose) {
  //Get packet into a buffer
  int sizePacket = dataAvailable();
  unsigned char buf[sizePacket];
  for (int i = 0; i < sizePacket; i++) {
    digitalWrite(SS_PIN,LOW);
    //  READ BYTE CMD
    int previous_cmd_status = SPI.transfer(ARDUINO_CMD_READ);
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    int shield_status = SPI.transfer(ARDUINO_CMD_AVAILABLE);
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    buf[i] = SPI.transfer(ARDUINO_CMD_AVAILABLE);
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    digitalWrite(SS_PIN,HIGH);
    delayMicroseconds(WAIT_TIME_BETWEEN_SPI_MSG);
  }
  
  if(verbose)
  {
    Serial.println("Packet received");
    Serial.println(hex8ToString(buf, sizePacket));
  }

  if(!_debug)
  {
    //Check crc
    int rc;
    int rspPktbufflen = 256;
    unsigned char rspPktBuff[rspPktbufflen];
  
    coap_packet_t pkt;
    coap_packet_t rsppkt; 
    size_t rsplen = sizeof(rspPktBuff);
    
    Serial.println(hex8ToString(&pkt.hdr.ver, 1));
  
    if (0 != (rc = coap_parse(&pkt, buf, sizePacket))){
      if(verbose)
      {
        Serial.print("Packet has a bad crc1: ");
        Serial.println(rc);
      }
      return "";
    }
  
    uint8_t scratch_raw[32];
    coap_rw_buffer_t scratch_buf = {scratch_raw, sizeof(scratch_raw)};
    coap_handle_req(&scratch_buf, &pkt, &rsppkt);    
    memset(rspPktBuff, 0, rspPktbufflen);
  
    if (0 != (rc = coap_build(rspPktBuff, &rsplen, &rsppkt))){
      if(verbose)
      {
        Serial.print("Packet has a bad crc2: ");
        Serial.println(rc);
      }
      return "";
    }
  
    //Send packet to Shield
    write(rspPktBuff,rsplen);
  }

  return hex8ToString(buf, sizePacket);
}

/**
 * \description: send a packet to the LoRa Network
 * \param: buff - the payload of the packet
 * \param: bufflen - the length of the payload
 */
void LoraShield::write(byte buff[], int bufflen) {
  digitalWrite(SS_PIN, LOW);

  // The byte is the status of the last command
  int previous_cmd_status = SPI.transfer(ARDUINO_CMD_WRITE); 
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
  
  // Data size to be sent
  // MSB
  int shield_status = SPI.transfer(bufflen >> 8);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);

  shield_status = SPI.transfer(bufflen);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
  
  //Send:  payload as bytes to send
  for (int i = 0; i < bufflen ; i++)
  {
    // written_data+1 as to avoid the first byte, which is the ARDUINO_CMD_WRITE
    shield_status = SPI.transfer(buff[i]);
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
  }

  digitalWrite(SS_PIN,HIGH); 
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
}

/**
 * \description: send a debug order to the contiki (to get all messages)
 * \param: getcontikidebug - if we want to receive all packets
 */
void LoraShield::getContikiDebug(bool getcontikidebug)
{
  _debug = getcontikidebug;
  digitalWrite(SS_PIN, LOW);
  int length = sizeof(0x01);

  // The byte is the status of the last command
  int previous_cmd_status = SPI.transfer(ARDUINO_CMD_DEBUG); 
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
  
  // Data size to be sent
  // MSB
  int shield_status = SPI.transfer(length >> 8);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);

  shield_status = SPI.transfer(length);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);

  shield_status = getcontikidebug ? SPI.transfer(0x01) : SPI.transfer(0x00);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);

  digitalWrite(SS_PIN,HIGH); 
  delay(WAIT_TIME_BETWEEN_SPI_MSG*3);
}

/**
 * \description: Change the frequency of the contiki shield
 * \param: freq - the new frequency
 */
void LoraShield::setFreq(long freq)
{
  digitalWrite(SS_PIN, LOW);
  int length = 0;

  if(freq < FREQ_MIN || freq > FREQ_MAX)
  {
    Serial.println("Error : Change to default frequency");
    freq = FREQ_DEFAULT;
  }
  else
    Serial.println("Change frequency : " + String(freq));

  // The byte is the status of the last command
  int previous_cmd_status = SPI.transfer(ARDUINO_CMD_FREQ); 
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
  
  // Data size to be sent
  // MSB
  int shield_status = SPI.transfer(length >> 8);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);

  shield_status = SPI.transfer(length);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);

  shield_status = SPI.transfer((freq>>24) & 0xFF);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
  shield_status = SPI.transfer((freq>>16) & 0xFF);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
  shield_status = SPI.transfer((freq>>8) & 0xFF);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
  shield_status = SPI.transfer((freq) & 0xFF);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);

  digitalWrite(SS_PIN,HIGH); 
  delay(WAIT_TIME_BETWEEN_SPI_MSG);
}

/**
 * \description: get the current frequency of the shield
 * \return: the new frequency
 */
unsigned long LoraShield::getFreq()
{
  digitalWrite(SS_PIN, LOW);
  unsigned long freq = 0;
  
  int previous_cmd_status = SPI.transfer(ARDUINO_CMD_GET_FREQ); 
  
  digitalWrite(SS_PIN,HIGH); 
  delay(WAIT_TIME_BETWEEN_SPI_MSG);
  
  while(dataAvailable() != 4)
  { }
  String freqString;
  int sizePacket = dataAvailable();
  for (int i = 0; i < sizePacket; i++) {
    digitalWrite(SS_PIN,LOW);
    //  READ BYTE CMD
    int previous_cmd_status = SPI.transfer(ARDUINO_CMD_READ);
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    int shield_status = SPI.transfer(ARDUINO_CMD_AVAILABLE);
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    String temp = String(SPI.transfer(ARDUINO_CMD_AVAILABLE), HEX);
    while(temp.length() < 2)
      temp = "0" + temp;
    freqString += temp;
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    digitalWrite(SS_PIN,HIGH);
    delayMicroseconds(WAIT_TIME_BETWEEN_SPI_MSG);
  }
  freq = (long)( strtol(freqString.substring(0,2).c_str(), NULL, 16) << 24
               | strtol(freqString.substring(2,4).c_str(), NULL, 16) << 16
               | strtol(freqString.substring(4,6).c_str(), NULL, 16) << 8
               | strtol(freqString.substring(6,8).c_str(), NULL, 16));
  return freq;
}

/**
 * \description: Change the bandwidth of the contiki shield
 * \param: bw - the new bandwidth
 */
void LoraShield::setBandwidth(int bw)
{
  digitalWrite(SS_PIN, LOW);
  int length = 0;
  if(bw < BW_MIN || bw > BW_MAX)
  {
    Serial.println("Error : Change to default BW");
    bw = BW_DEFAULT;
  }
  else
    Serial.println("Change bandwidth mode : " + String(bw));

  // The byte is the status of the last command
  int previous_cmd_status = SPI.transfer(ARDUINO_CMD_BW_CFG); 
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
  
  // Data size to be sent
  // MSB
  int shield_status = SPI.transfer(length >> 8);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);

  shield_status = SPI.transfer(length);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);

  shield_status = SPI.transfer(bw);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);

  digitalWrite(SS_PIN,HIGH); 
  delay(WAIT_TIME_BETWEEN_SPI_MSG);
}

/**
 * \description: get the current bandwidth of the shield
 * \return: the bandwidth
 */
int LoraShield::getBandwidth()
{
  digitalWrite(SS_PIN, LOW);
  int bw = 0;
  
  int previous_cmd_status = SPI.transfer(ARDUINO_CMD_GET_BW_CFG); 
  
  digitalWrite(SS_PIN,HIGH); 
  delay(WAIT_TIME_BETWEEN_SPI_MSG);
  
  while(!dataAvailable())
  {}
  
  int sizePacket = dataAvailable();
  for (int i = 0; i < sizePacket; i++) {
    digitalWrite(SS_PIN,LOW);
    //  READ BYTE CMD
    int previous_cmd_status = SPI.transfer(ARDUINO_CMD_READ);
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    int shield_status = SPI.transfer(ARDUINO_CMD_AVAILABLE);
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    bw += (SPI.transfer(ARDUINO_CMD_AVAILABLE) << 8*(sizePacket-1-i));
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    digitalWrite(SS_PIN,HIGH);
    delayMicroseconds(WAIT_TIME_BETWEEN_SPI_MSG);
  }
  
  return bw;
}

/**
 * \description: Change the coding rate of the contiki shield
 * \param: cr - the new coding rate
 */
void LoraShield::setCodingRate(int cr)
{
  digitalWrite(SS_PIN, LOW);
  int length = 0;
  if(cr < CR_MIN || cr > CR_MAX)
  {
    Serial.println("Error : Change to default CR");
    cr = CR_DEFAULT;
  }
  else
    Serial.println("Change code rate : " + String(cr));

  // The byte is the status of the last command
  int previous_cmd_status = SPI.transfer(ARDUINO_CMD_CR_CFG); 
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
  
  // Data size to be sent
  // MSB
  int shield_status = SPI.transfer(length >> 8);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);

  shield_status = SPI.transfer(length);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);

  shield_status = SPI.transfer(cr);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);

  digitalWrite(SS_PIN,HIGH); 
  delay(WAIT_TIME_BETWEEN_SPI_MSG);
}

/**
 * \description: get the current coding rate of the shield
 * \return: the coding rate
 */
int LoraShield::getCodingRate()
{
  digitalWrite(SS_PIN, LOW);
  int cr = 0;
  
  int previous_cmd_status = SPI.transfer(ARDUINO_CMD_GET_CR_CFG); 
  
  digitalWrite(SS_PIN,HIGH); 
  delay(WAIT_TIME_BETWEEN_SPI_MSG);
  
  while(!dataAvailable())
  {}
  
  int sizePacket = dataAvailable();
  for (int i = 0; i < sizePacket; i++) {
    digitalWrite(SS_PIN,LOW);
    //  READ BYTE CMD
    int previous_cmd_status = SPI.transfer(ARDUINO_CMD_READ);
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    int shield_status = SPI.transfer(ARDUINO_CMD_AVAILABLE);
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    cr += (SPI.transfer(ARDUINO_CMD_AVAILABLE) << 8*(sizePacket-1-i));
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    digitalWrite(SS_PIN,HIGH);
    delayMicroseconds(WAIT_TIME_BETWEEN_SPI_MSG);
  }
  
  return cr;
}

/**
 * \description: Change the spreading factor of the contiki
 * \param: sf - the new spreading factor
 */
void LoraShield::setSpreadingFactor(int sf)
{
  digitalWrite(SS_PIN, LOW);
  int length = 0;
  if(sf < SF_MIN || sf > SF_MAX)
  {
    Serial.println("Error : Change to default SF");
    sf = SF_DEFAULT;
  }
  else
    Serial.println("Change spreading factor : " + String(sf));

  // The byte is the status of the last command
  int previous_cmd_status = SPI.transfer(ARDUINO_CMD_SF_CFG); 
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
  
  // Data size to be sent
  // MSB
  int shield_status = SPI.transfer(length >> 8);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);

  shield_status = SPI.transfer(length);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);

  shield_status = SPI.transfer(sf);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);

  digitalWrite(SS_PIN,HIGH); 
  delay(WAIT_TIME_BETWEEN_SPI_MSG);
}

/**
 * \description: get the contiki spreading factor
 * \return: the spreading factor value
 */
int LoraShield::getSpreadingFactor()
{
  digitalWrite(SS_PIN, LOW);
  int sf = 0;
  
  int previous_cmd_status = SPI.transfer(ARDUINO_CMD_GET_SF_CFG); 
  
  digitalWrite(SS_PIN,HIGH); 
  delay(WAIT_TIME_BETWEEN_SPI_MSG);
  
  while(!dataAvailable())
  {}
  
  int sizePacket = dataAvailable();
  for (int i = 0; i < sizePacket; i++) {
    digitalWrite(SS_PIN,LOW);
    //  READ BYTE CMD
    int previous_cmd_status = SPI.transfer(ARDUINO_CMD_READ);
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    int shield_status = SPI.transfer(ARDUINO_CMD_AVAILABLE);
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    sf += (SPI.transfer(ARDUINO_CMD_AVAILABLE) << 8*(sizePacket-1-i));
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    digitalWrite(SS_PIN,HIGH);
    delayMicroseconds(WAIT_TIME_BETWEEN_SPI_MSG);
  }
  
  return sf;
}

/**
 * \description: Change the whole confiuration (spreading factor, coderate, bandwidth)
 * \param: the config (see the full doc for more details)
 */
void LoraShield::setRFConfig(int rfconfig)
{
  digitalWrite(SS_PIN, LOW);
  int length = 0;
  if(rfconfig < CONF_MIN || rfconfig > CONF_MAX)
  {
    Serial.println("Error : Change to default rf config");
    rfconfig = CONF_DEFAULT;
  }
  else
    Serial.println("Change config mode : " + String(rfconfig));
    
  // The byte is the status of the last command
  int previous_cmd_status = SPI.transfer(ARDUINO_CMD_RF_CFG); 
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
  
  // Data size to be sent
  // MSB
  int shield_status = SPI.transfer(length >> 8);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
  shield_status = SPI.transfer(length);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);

  shield_status = SPI.transfer(rfconfig);
  delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);

  digitalWrite(SS_PIN,HIGH); 
  delay(WAIT_TIME_BETWEEN_SPI_MSG);
}

/**
 * \description: get the current coding rate of the shield
 * \return: the coding rate
 */
String LoraShield::getMAC()
{
  digitalWrite(SS_PIN, LOW);
  uint8_t MAC[8];
  
  int previous_cmd_status = SPI.transfer(ARDUINO_CMD_GET_MAC); 
  
  digitalWrite(SS_PIN,HIGH); 
  delay(WAIT_TIME_BETWEEN_SPI_MSG);
  
  while(!dataAvailable())
  {}
  
  int sizePacket = dataAvailable();
  for (int i = 0; i < sizePacket; i++) {
    digitalWrite(SS_PIN,LOW);
    //  READ BYTE CMD
    int previous_cmd_status = SPI.transfer(ARDUINO_CMD_READ);
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    int shield_status = SPI.transfer(ARDUINO_CMD_AVAILABLE);
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    MAC[i] = SPI.transfer(ARDUINO_CMD_AVAILABLE);
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    digitalWrite(SS_PIN,HIGH);
    delayMicroseconds(WAIT_TIME_BETWEEN_SPI_MSG);
  }
  
  return hex8ToString(MAC, 8);
}

/**
 * \description: prints 8-bit data in hex
 * \param: data - 8-bit data
 * \param: length - the length of the data
 * \return: the String of the hex value of data
 */
String LoraShield::hex8ToString(uint8_t *data, uint8_t length)
{
  char tmp[length*2+1];
  byte first ;
  int j=0;
  for (uint8_t i=0; i<length; i++) 
  {
    first = (data[i] >> 4) | 48;
    if (first > 57) tmp[j] = first + (byte)39;
    else tmp[j] = first ;
    j++;

    first = (data[i] & 0x0F) | 48;
    if (first > 57) tmp[j] = first + (byte)39; 
    else tmp[j] = first;
    j++;
  }
  tmp[length*2] = 0; // Null terminated
  return String(tmp);
}
