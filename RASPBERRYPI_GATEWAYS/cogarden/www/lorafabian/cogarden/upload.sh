#!/bin/sh
wget "https://dweet.io/dweet/for/basilic?$(cut /var/www/lorafabian/cogarden/received_msg2.txt -b 12-)" -O /dev/null

