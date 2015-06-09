void setup() {
  LoRa_init();
  LoRa_RF_config(4);
//  LoRa_freq_channel(3);


  // Warning always perform Serial.begin after LoRa_init
  Serial.begin(9600);
}
int cnt =0;
// Send Example


/*
void loop () {
  
  String msg;
  msg ="LoraFabian:test";
  
  msg  =  msg + cnt;
  
  LoRa_send(msg);
cnt++;
  delay(10000);

}

*/
// Receive example
void loop () {
  int avail;
  byte c;

  avail = LoRa_available();

  if (avail){
    Serial.print(avail);
    Serial.print(" bytes received:");
    // read bytes one by one
    for (int i=0; i<avail; i++){
      c = LoRa_read();
      Serial.write(c);

    }
    Serial.println("");
  }

  delay(100);
}


