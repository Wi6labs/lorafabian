This document explains how to create a LoRa gateway with a RaspberryPi and a Froggy Factory LoRa shield.

Warning: At now this method does not support RaspberryPi 2.

- At first you need a "Raspberry Pi to Arduino shields connection bridge" board to connect RPi to the LoRa shield.
You can order one at http://www.cooking-hacks.com/documentation/tutorials/raspberry-pi-to-arduino-shields-connection-bridge

- Then install raspbian on your RPi (tested with version "Linux raspberrypi 3.18.7+ #755 PREEMPT Thu Feb 12 17:14:31 GMT 2015 armv6l")

- Install apache and php with commands:
sudo apt-get update
sudo apt-get install apache2
sudo apt-get install php5

- Then copy reference project LoraWebServerRPI/ on your Rpi
lorafabian/ into /home/pi
www/lorafabian/ inot /var/www

Warning: Be careful of access rights given to www/
