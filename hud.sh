#!/bin/bash
set -euo pipefail

# Setup TTY for Arduino
# see https://playground.arduino.cc/Interfacing/LinuxTTY/
stty -F /dev/ttyACM2 cs8 9600 ignbrk \
    -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts

tail -f /dev/ttyACM2 \
    | ruby hud.rb \
    | osd_cat --align left --pos bottom --lines 1 --outline 1 --color white --font -bitstream-*-medium-r-*-*-*-120-*-*-m-*-*-0 # see xfontsel
