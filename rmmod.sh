#!/bin/sh

rmmod a333
#j001+s
#rmmod dvb-usb
#rmmod dvb-pll
#rmmod dvb-core
modprobe -r dvb-usb
modprobe -r dvb-pll
modprobe -r dvb-core
#j001+e
