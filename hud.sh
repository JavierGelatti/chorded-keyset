#!/bin/bash
set -euo pipefail

# Setup TTY for Arduino
# see https://playground.arduino.cc/Interfacing/LinuxTTY/
stty -F /dev/ttyACM0 cs8 9600 ignbrk \
    -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts

cat /dev/ttyACM0 \
    | ruby hud.rb \
    | osd_cat --align left --pos bottom --lines 1 --outline 1 --color white --font -bitstream-*-medium-r-*
