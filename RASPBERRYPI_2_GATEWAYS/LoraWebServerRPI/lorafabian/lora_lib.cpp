/*--------------------------------------------------------------------------

              j]_                   .___
._________    ]0Mm                .=]MM]=
M]MM]MM]M]1  jMM]P               d]-' NM]i
-~-~   4MM1  d]M]1              d]'   jM]'
       j]MT .]M]01       d],  .M]'    d]#
       d]M1 jM4M]1  .,  d]MM  d]I    .]M'
       ]0]  M/j]0(  d]L NM]f d]P     jM-
       M]M .]I]0M  _]MMi -' .]M'
       M]0 jM MM]  jM-M>   .]M/
       ]0F MT ]M]  M>      d]M1        .,
      j0MT.]' M]M j]1 .mm .]MM ._d]_,   J,
      jM]1jM  ]01 =] .]M/ jM]Fd]M]MM]   .'
      j]M1#T .M]1.]1 jM]' M]0M/^ "M]MT  j         .",    .__,  _,-_
      jMM\]' J]01jM  M]M .]0]P    ]0]1  i         1 1   .'  j .'  "1
      j]MJ]  jM]1]P .]M1 jMMP     MM]1  I        J  t   1   j J    '
      =M]dT  jM]q0' dM]  M]MT     ]MM  j        j   j  j    J 1
      ]M]M`  j]0j#  ]MF  ]M]'    .M]P  J       .'   j  J  .J  4_,
      M]0M   =MM]1 .M]'  MM]     jM](  1       r    j  1  --,   "!
      ]0MT   ]M]M  jM@   ]M]     M]P  j       J     j j     4     1
      MM]'   M]0P  j]1  .M]M    j]M'  J      j'     ",?     j     1
     _]M]    M]0`  jM1 .MNMM,  .]M'   1     .'       11     1    j'
     jM]1   jM]@   j]L_]'?M]M__MP'    \     J        1G    J    .'
     j]0(   jM]1   "M]P'  "N]M/-      "L__J L________'?L__- *__,'
     "-'    "--

----------------------------------------------------------------------------

Copyright (c) <2014>, <Wi6labs>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the wi6labs nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL WI6LABS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

-----------------------------------------------------------------------------*/

#include "arduPi.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
 #include <arpa/inet.h>

const int slaveSelectPin = 10;
const byte ResetPin          = 5; 

void LoRa_init() {
  // Reset The LoRa modem
  pinMode (ResetPin, OUTPUT);
  digitalWrite(ResetPin,HIGH); 
  delay(100);
  digitalWrite(ResetPin,LOW); 
  delay(50);
  digitalWrite(ResetPin,HIGH); 
  // Let it wake up
  delay(500);

  // Set NSS to high
  pinMode (slaveSelectPin, OUTPUT);
  digitalWrite(slaveSelectPin,HIGH);

  // initialize SPI:
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0) ;
  SPI.setClockDivider(SPI_CLOCK_DIV2048); // 125kHz

  // Wait SPI to be ready
  delay(500);
}

void LoRa_send(unsigned char *in, int len) {

 digitalWrite(slaveSelectPin,LOW);
  delay(1);

  //  send data
  SPI.transfer(0x02);
  delay(1);
  SPI.transfer(len >> 8);
  delay(1);
  SPI.transfer(len & 0xFF);
  delay(1);
  for (int i=0; i<len; i++) {
//    printf("%d", i);
    SPI.transfer(in[i]);
    delay(1);
  }


  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH);

  delay(100);
}

int LoRa_available(){
  int available_msb;
  int available_lsb;
  int available;

  digitalWrite(slaveSelectPin,LOW);
  delay(1);

  //  data available cmd
  SPI.transfer(0x00);
  delay(1);
  SPI.transfer(0x00);
  delay(1);
  available_msb = SPI.transfer(0x00);
  delay(1);
  available_lsb = SPI.transfer(0x00);
  delay(1);

  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH);

  delay(1);

  available = (available_msb<<8) + available_lsb;

  return available;
}

char LoRa_read() {

  byte c;
  digitalWrite(slaveSelectPin,LOW);
  delay(1);

  //  read data
  SPI.transfer(0x01);
  delay(1);
  SPI.transfer(0x00);
  delay(1);
  c = SPI.transfer(0x00);
  delay(1);


  digitalWrite(slaveSelectPin,HIGH);
  delay(1);

  return c;

}

// Change radio configuration: 0 to 3
// 0 is the fastest but less robust
// 3 is the slowest but more robust (better distance)
void LoRa_RF_config(int in) {

  digitalWrite(slaveSelectPin,LOW);
  delay(1);
  // CFG cmd
  SPI.transfer(0x05);
  delay(1);
  SPI.transfer(0x00);
  delay(1);
  SPI.transfer(0x01);
  delay(1);
  SPI.transfer(in);
  delay(1);

  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH);

  delay(100);
}

// Change radio frequency
// input is in Hz
// Range is 863 000 000 to 870 000 000
// Be careful of the band width used at band limits
void LoRa_freq(long f) {
  digitalWrite(slaveSelectPin,LOW);
  delay(1);
  //  FREQ cmd
  SPI.transfer(0x04);
  delay(1);
  SPI.transfer(0x00);
  delay(1);
  SPI.transfer(0x04);
  delay(1);
  SPI.transfer((f>>24) & 0xFF);
  delay(1);
  SPI.transfer((f>>16) & 0xFF);
  delay(1);
  SPI.transfer((f>>8) & 0xFF);
  delay(1);
  SPI.transfer(f & 0xFF);
  delay(1);

  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH);

  delay(100);

}


// Pre implmented radio frequency channel configuration
// input is channel number
void LoRa_freq_channel(int nb) {
  switch(nb) {
    case 0:
      LoRa_freq(868000000);
    break;
    case 1:
      LoRa_freq(868100000);
    break;
    case 2:
      LoRa_freq(868200000);
    break;
    case 3:
      LoRa_freq(868300000);
    break;
    case 4:
      LoRa_freq(869500000);
    break;
    case 5:
      LoRa_freq(869600000);
    break;
    case 6:
      LoRa_freq(869800000);
    break;
    case 7:
      LoRa_freq(869900000);
    break;
  }
}


// Get Signal to Noise Ratio from la message received
int LoRa_last_snr(){
  char snr;

  //  read SNR
  digitalWrite(slaveSelectPin,LOW);
  delay(1);

  SPI.transfer(0x06);
  delay(1);
  SPI.transfer(0x00);
  delay(1);
  snr = SPI.transfer(0x00);
  delay(1);

  digitalWrite(slaveSelectPin,HIGH);

  delay(100);

  return int(snr);
}

// Get Received signal strength indication from la message received
int LoRa_last_rssi(){
  char rssi;
  //  read RSSI
  digitalWrite(slaveSelectPin,LOW);
  delay(1);

  SPI.transfer(0x07);
  delay(1);
  SPI.transfer(0x00);
  delay(1);
  rssi = SPI.transfer(0x00);
  delay(1);

  digitalWrite(slaveSelectPin,HIGH);

  delay(100);

  return int(rssi);
}

// Set Spreading Factor
void LoRa_SF(int in) {
  digitalWrite(slaveSelectPin,LOW);
  delay(1);
  
  // CFG cmd
  SPI.transfer(0x08);
  delay(1);  
  SPI.transfer(0x00);
  delay(1);
  SPI.transfer(0x01);
  delay(1);
  SPI.transfer(in);
  delay(1);
  
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH); 

  delay(100);
}

// Set Band Width
void LoRa_BW(int in) {
  digitalWrite(slaveSelectPin,LOW);
  delay(1);
  
  // CFG cmd
  SPI.transfer(0x09);
  
  delay(1);  
  SPI.transfer(0x00);
  delay(1);
  SPI.transfer(0x01);
  delay(1);
  SPI.transfer(in);
  delay(1);
  
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH); 

  delay(100);
}

// Set Coding Rate
void LoRa_CR(int in) {


  digitalWrite(slaveSelectPin,LOW);
  delay(1);
  // CFG cmd
  SPI.transfer(0x0A);
  delay(1);  
  SPI.transfer(0x00);
  delay(1);
  SPI.transfer(0x01);
  delay(1);
  SPI.transfer(in);
  delay(1);
  
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH); 

  delay(100);
}

// Set TX Power
void LoRa_TX_Power(int in) {

  digitalWrite(slaveSelectPin,LOW);
  delay(1);
  // CFG cmd
  SPI.transfer(0x0B);
  delay(1);  
  SPI.transfer(0x00);
  delay(1);
  SPI.transfer(0x01);
  delay(1);
  SPI.transfer(in);
  delay(1);
  
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH); 

  delay(100);
} 


