//#include <AirQuality.h>
#include "Arduino.h"
#include <math.h>
#include "DHT.h"
#define DHTPIN A2
#include <SerialLCD.h>
#include <SoftwareSerial.h> //this is a must
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

SerialLCD slcd(6,7);  
//AirQuality airqualitysensor;
//int current_quality =-1;
int pin=5;

void setup() {
  float Rsensor_light;
  int sensorValue_hydro = 0;
  
   LoRa_init();
  LoRa_RF_config(4); slcd.begin();
  
  dht.begin();
  pinMode(A0, OUTPUT);  
  slcd.print("hello, world!");
 // airqualitysensor.init(14);
 
 Serial.begin(9600);
 }

void loop() {
 String msg;
 int light_sensorvalue = analogRead(A0);                         // value of light_sensor
int Rsensor=(float)(1023-light_sensorvalue)*10/light_sensorvalue;          // resistance ?
Serial.print("Rsensor");
Serial.println(Rsensor);
 
  int sensorValue_hydro = analogRead(A1);            // Value of hydrometrie
  
 float h = dht.readHumidity();
 float t = dht.readTemperature();             // value temperature et humidity


  //current_quality=airqualitysensor.slope();   // value of airquality; value return = 1 hight pollu, 3 fresh air;;
   
 Serial.print("ligth");
 Serial.println(light_sensorvalue);
 
 Serial.print("hydro");
 Serial.println(sensorValue_hydro);
 
 Serial.print("humidity");
 Serial.println(h);
 
 Serial.print("temperature");
 Serial.println(t);

 //Serial.print("air quality");
 //Serial.println(current_quality);
  Serial.println("-----------------------------------");
  msg  = "LoraFabian:{";

  msg += "\"light\":\"";
  msg +=   light_sensorvalue;
  msg += "\",";

  msg += "\"hydro\":\"";
  msg +=   sensorValue_hydro;
  msg += "\",";

  msg += "\"humidity\":\"";
  msg +=   h;
  msg += "\",";

  msg += "\"temperature\":\"";
  msg +=   t;
  
  
  msg += "\"}";


  
  
  Serial.println(msg);
  Serial.println("-----------------------------------"); 

   LoRa_send(msg);

 delay(30000);
 }

