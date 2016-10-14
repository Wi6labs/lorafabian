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

//Include arduPi library
#include <stdlib.h>
#include <stdio.h>
#include "arduPi.h"
#include <time.h>
#include "lora_lib.h"

#define MAX_STRING 255
#define SEND_MSG_FILE "/var/www/lorafabian/msg2send.txt"
#define RECEIVED_MSG "/var/www/lorafabian/received_msg.txt"

void setup() 
{
  LoRa_init();
  LoRa_RF_config(0);
}

// Check if a message to send is available
void checkRequest() {
  time_t rawtime;
  struct tm * timeinfo;
  FILE* file_pt = NULL;
  char str[MAX_STRING] = "";
  
  // open the file which contains the message to send
  file_pt = fopen(SEND_MSG_FILE, "r");
  
  // if file exists, send its content
  if (file_pt != NULL)
  {
     // read the message on the first line of the file
    fgets(str, MAX_STRING, file_pt); 
    // close file
    fclose(file_pt);
    // and delete it
    remove(SEND_MSG_FILE);
    
    // send the message
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    printf ( "\n%s", asctime (timeinfo) );
    printf("Send Lora msg : \"%s\"\n",str);
    LoRa_send(str, strlen(str) );

  }
}



// manage a received Lora message
void treatMsg(uint8_t *data,int len)
{
  FILE *fp;

  // add null character
  data[len]=0;

  printf("Lora received msg : \"%s\"\n",(char*)data);
  fp=fopen(RECEIVED_MSG, "a");
  // copy the message at the end of the file received_msg.txt
  fprintf(fp, "%s\n",data);
  fclose(fp);
}


// the loop to receive messages
void loop() {
  int previous_cmd_status;
  int shield_status;
  int available_msb;
  int available_lsb;
  int available;
  int read_data;
  int i;
  time_t rawtime;
  struct tm * timeinfo;
  uint8_t data[MAX_STRING];

  available = LoRa_available();

  if (available) 
  {

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    printf ("\n%s", asctime (timeinfo) );
    printf("Message received, size=%d\n",available);
    delay(1);

    // loop for reading data
    for (i=0; i<available; i++) 
    {
      data[i]=LoRa_read();
    }
    treatMsg(data,available);
  }
  delay(100);
}


int main (){
  printf("Lora gateway running...\n");
  setup();
  while(1){
    loop();
    checkRequest();
  }
  return (0);
}
