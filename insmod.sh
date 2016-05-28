#!/bin/sh

modprobe dvb-core
modprobe dvb-pll
modprobe dvb-usb


insmod ./a867.ko debug=255

