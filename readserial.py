#!/usr/bin/python
# -*- coding: utf-8 -*-
# Reading the serial data in realtime

import serial

ser = serial.Serial("/dev/ttyACM0", 115200)
while True:
    print ser.readline()
