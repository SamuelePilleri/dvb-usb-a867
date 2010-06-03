#!/bin/sh

modprobe i2c-core

#j001+s
#insmod ./dvb-core.ko
#insmod ./dvb-pll.ko
#insmod ./dvb-usb.ko debug=255
modprobe dvb-core
modprobe dvb-pll
modprobe dvb-usb
#j001+e

insmod ./a333.ko debug=255

