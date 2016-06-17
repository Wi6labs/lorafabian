void setup() {
  LoRa_init();
//  LoRa_RF_config(1);
 // LoRa_freq_channel(3);

  // Advanced LoRa user parameters :

  // frequency
//  LoRa_freq(868100000);
  
  // Spreading factor
  // LoRa_SF(12);      // [7: SF7,
                      //  8: SF8,
                      //  9: SF9,
                      // 10: SF10,
                      // 11: SF11,
                      // 12: SF12]

  // Band Width
  // LoRa_BW(0);       // [0: 125 kHz,
                      //  1: 250 kHz,
                      //  2: 500 kHz]

  // Coding rate
  //LoRa_CR(1);       // [1: 4/5,
                      //  2: 4/6,
                      //  3: 4/7,
                      //  4: 4/8]

  // TX power
  //LoRa_TX_Power(14);   // in dBm, range: [2:14]

  // Warning always perform Serial.begin after LoRa_init
  Serial.begin(9600);

}
/*
// Send Example
void loop () {
  LoRa_send("hehehe");

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
    
   Serial.print("SNR : ");
  Serial.print ( LoRa_last_snr() );
  Serial.print(" RSSI : ");
  Serial.println( LoRa_last_rssi() );
  }

  delay(100);
}


