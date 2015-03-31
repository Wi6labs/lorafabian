This document explains how to create a LoRa gateway with a RaspberryPi and a Froggy Factory LoRa shield.

Warning: At now this method does not support RaspberryPi 2. But we are working on it.

- At first you need a "Raspberry Pi to Arduino shields connection bridge" board to connect RPi to the LoRa shield.
You can order one at http://www.cooking-hacks.com/documentation/tutorials/raspberry-pi-to-arduino-shields-connection-bridge

- Then install raspbian on your RPi (tested with version "Linux raspberrypi 3.18.7+ #755 PREEMPT Thu Feb 12 17:14:31 GMT 2015 armv6l")

- Install apache and php with commands:
sudo apt-get update
sudo apt-get install apache2
sudo apt-get install php5

- Then copy reference project LoraWebServerRPI/ on your Rpi
lorafabian/ into /home/pi
www/lorafabian/ into /var/www

Warning: Be careful of access rights given to www/ because you will access from /home/pi
Perform the following command to give full access: sudo chmod 777 www 

- Compile the gateway software running the following script
~/lorafabian/compile.sh

-Then run it with super user privilege (this is mandatory because SPI bus is accessed): sudo ./lorafabian
Note: The gateway software is not started at RPi startup, but you can do it.

HOW IT WORKS:
------------

If you succeed with above steps, the gateway is now running.
This gateway is mono-channel.
The default channel is:
- 868.1 MHz
- Spreading Factor 7
- Band width 125 kHz
- Coding Rate 4/5

Every Froggy Factory LoRa shield are configured by default with this setup.

To discard messages from other networks, input packet are filtered.
Only message starting with "LoraFabian:" will be received by this gateway.

There is a web server running on the RPi. You can access it on your local network with the link:
192.168.xxx.xxx/lorafabian

On this page you can send message or check received one.

Have fun!




