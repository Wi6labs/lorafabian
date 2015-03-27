#include <DHT11.h>
#include <AirQuality.h>
#include "Arduino.h"
#include <math.h>
#include <SerialLCD.h>
#include <SoftwareSerial.h> //this is a must


SerialLCD slcd(11,12);  
AirQuality airqualitysensor;
int current_quality =-1;
int pin=4;
DHT11 dht11(pin); 

void setup() {
  float Rsensor_light;
  int sensor_Pin_hydro = A1;
  int sensorValue_hydro = 0;
  byte humidity, temperature;
  slcd.begin();
  slcd.print("hello, world!");
  airqualitysensor.init(14);
}

void loop() {
  int light_sensorvalue = analogRead(A0);                         // value of light_sensor
//  Rsensor=(float)(1023-sensorValue)*10/sensorValue;          // resistance ?
 
 
  int sensorValue_hydro = analogRead(A1);            // Value of hydrometrie
  
  float humidity, temperature;
  dht11.read(humidity, temperature);               // value temperature et humidity


  current_quality=airqualitysensor.slope();   // value of airquality; value return = 1 hight pollu, 3 fresh air;;
}

