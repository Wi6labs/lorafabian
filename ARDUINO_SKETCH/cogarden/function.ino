//#include <math.h>
//#include <SerialLCD.h>
//#include <SoftwareSerial.h> //this is a must
//#include <DHT.h>
//#include "AirQuality.h"
//#include "Arduino.h"
//
//byte readDHT11(byte pin, byte* temperature, byte* humidity) {
// 
//  /* data[] -> buffer contenant les données du cpateur
//   * counter -> compteur permettant de savoir quel bit est reçu (bitwise)
//   * index -> compteur permettant de savoir quel octet est reçu (bitwise)
//   * timeout -> compteur pour le timeout
//   */
//  byte data[5] = { 0 }, counter = 7, index = 0;
//  unsigned int timeout;
// 
//  /* Conversion du numéro de broche Arduino en ports/masque binaire "bas niveau" */
//  /* Utiliser les registres du microcontrôleur est bien plus rapide que digitalWrite() */
//  uint8_t bit = digitalPinToBitMask(pin);
//  uint8_t port = digitalPinToPort(pin);
//  volatile uint8_t *ddr = portModeRegister(port); // Registre MODE (INPUT / OUTPUT)
//  volatile uint8_t *out = portOutputRegister(port); // Registre OUT (écriture)
//  volatile uint8_t *in = portInputRegister(port); // Registre IN (lecture)
// 
//  /* Réveil du capteur */
//  *ddr |= bit; // OUTPUT
//  *out &= ~bit; // LOW
//  delay(18); // Temps d'attente à LOW causant le réveil du capteur
//  *out |= bit; // HIGH
//  delayMicroseconds(40);
//  *ddr &= ~bit; // INPUT
// 
//  /* Attente de la réponse du capteur */
//  timeout = 0;
//  while(!(*in & bit)) /* Attente d'un état LOW */
//    if (++timeout == 10000)
//      return (DHT11_TIMEOUT_ERROR);
//  timeout = 0;
//  while(*in & bit) /* Attente d'un état HIGH */
//    if (++timeout == 10000)
//      return DHT11_TIMEOUT_ERROR;
// 
//  /* Lecture des données du capteur (40 bits) */
//  for (byte i = 0; i < 40; ++i) {
// 
//    /* Attente d'un état LOW */
//    timeout = 0;
//    while(!(*in & bit))
//      if (++timeout == 10000)
//        return DHT11_TIMEOUT_ERROR;
// 
//    /* Mise en mémoire du temps courant */
//    unsigned long t = micros();
// 
//    /* Attente d'un état HIGH */
//    timeout = 0;
//    while(*in & bit)
//      if (++timeout == 10000)
//        return DHT11_TIMEOUT_ERROR;
// 
//    /* Si le delta Temps est supérieur à 40µS c'est un "1", sinon c'est un "0" */
//    if ((micros() - t) > 40)
//      data[index] |= (1 << counter); // "1"
//    // Le tableau data[] est initialisé à "0" par défaut <span class="wp-smiley wp-emoji wp-emoji-wink" title=";)">;)</span>
// 
//    /* Si le compteur de bits atteint zéro */
//    if (counter-- == 0) {
//      counter = 7; /* On passe à l'octet suivant */
//      ++index;  
//    }
//  }
// 
//  /* Format des données :
//   * [0] = humidité en %
//   * [1] = zéro
//   * [2] = température en degrés Celsius
//   * [3] = zéro
//   * [4] = checksum (humidité + température)
//   */
//  *humidity = data[0];
//  *temperature = data[2];
// 
//  /* Vérifie la checksum */
//  if (data[4] != (data[0] + data[2]))
//    return DHT11_CHECKSUM_ERROR; /* Erreur de checksum */
//  else
//    return DHT11_SUCCESS; /* Pas d'erreur */
//}
