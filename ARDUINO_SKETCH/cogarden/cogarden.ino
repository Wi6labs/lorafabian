//#include <AirQuality.h>
#include "Arduino.h"
#include <math.h>
#include "DHT.h"
#define DHTPIN A2
//#include <SerialLCD.h>
#include <SoftwareSerial.h> //this is a must
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

//SerialLCD slcd(6,7);  
//AirQuality airqualitysensor;
//int current_quality =-1;
int pin=5;

void setup() {
  int sensorValue_hydro = 0;
  
   LoRa_init();
  LoRa_RF_config(4); 
  
  //slcd.begin();
  
  dht.begin();
  //slcd.print("hello, world!");
 // airqualitysensor.init(14);
 
 Serial.begin(9600);
 }

void loop() {
 String msg;
 led();
 int light_sensorvalue = analogRead(A0); // value of light_sensor
// delay(30000);
 Serial.print("light");
 Serial.println(light_sensorvalue);
 //double voltage = (5 / 1024)* light_sensorvalue;
 //double r = (20 * voltage)/(5 - voltage);
 //Serial.print("voltage");
  //Serial.println(voltage);
 //Serial.print("r");
//Serial.println(r);
 //int lux = (int )(500 / r);
// float Vout = light_sensorvalue*0.0048828125;
//int lux = ((500*(5-Vout))/(10*Vout));
 //int lux = light_sensorvalue * 100;
  int sensorValue_hydro = analogRead(A1);            // Value of hydrometrie
  
 float h = dht.readHumidity();
 float t = dht.readTemperature();             // value temperature et humidity

aff_panel(light_sensorvalue, h, sensorValue_hydro, t);
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
  msg  = "LoraFabian:";

  msg += "light=";
  msg +=   light_sensorvalue;
  msg += "&";

  msg += "hydro=";
  msg +=   sensorValue_hydro;
  msg += "&";

  msg += "humidity=";
  msg +=   h;
  msg += "&";

  msg += "temperature=";
  msg +=   t;
  
  
 // msg += "";


  
  
  Serial.println(msg);
  Serial.println("-----------------------------------"); 

   LoRa_send(msg);

 delay(30000);
 }

