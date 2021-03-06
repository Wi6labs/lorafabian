This document explains how to create a LoRa gateway with a RaspberryPi 3 and a Froggy Factory LoRa shield.


- At first you need a "Raspberry Pi to Arduino shields connection bridge" board to connect RPi to the LoRa shield.
You can order one at http://www.cooking-hacks.com/documentation/tutorials/raspberry-pi-to-arduino-shields-connection-bridge

On this setting IOREF pin is floating on the shield. So you have to connect it to 3V3 with a single wire. Look at "RPi GW IOREF 3V3.jpg" in this github directory for illustration.

- Then install raspbian on your RPi (tested with version "Linux raspberrypi 4.4.21-v7+ #911 SMP Thu Sep 15 14:22:38 BST 2016 armv7l GNU/Linux")

- Enable SPI with command: sudo raspi-config
  9 Advanced Options > A6 SPI > 
     Would you like the SPI interface to be enabled? > Yes
  for more info about SPI activation: https://www.raspberrypi.org/documentation/hardware/raspberrypi/spi/README.md

- Install apache and php with commands:
sudo apt-get update
sudo apt-get install apache2
sudo apt-get install php5

- Then copy reference project LoraWebServerRPI/ on your Rpi
lorafabian/ into /home/pi
www/lorafabian/ into /var/www/html

Warning: Be careful of access rights given to www/ and www/html/lorafabian  because you will access from /home/pi
Perform the following commands to give full access: sudo chmod 777 /var/www;  sudo chmod 777 /var/www/html; sudo chmod 777 /var/www/html/lorafabian;  sudo chmod 777 /var/www/html/lorafabian/*; 

- Give exectution right to Gateway compilation script:
chmod 777 ~/lorafabian/compile.sh
- Compile the gateway software running the following script
cd ~/lorafabian/
./compile.sh

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

There is a web server running on the RPi. You can access it on your local network with the link:
192.168.xxx.xxx/lorafabian

On this page you can send message or check received one.

Have fun!




