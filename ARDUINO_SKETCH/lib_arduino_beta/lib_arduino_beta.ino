void setup() {
  LoRa_init();
  LoRa_RF_config(1);

  // Warning always perform Serial.begin after LoRa_init
  Serial.begin(9600);
}

// Send Example
void loop () {
  LoRa_send("hehehe");

  delay(3000);

}

/*
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

*/
