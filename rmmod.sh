#!/bin/sh

rmmod a867

modprobe -r dvb-usb
modprobe -r dvb-pll
modprobe -r dvb-core

