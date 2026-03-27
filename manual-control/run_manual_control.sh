#!/bin/bash

# compile the code with 
arduino-cli compile --fqbn arduino:avr:mega --build-property "compiler.cpp.extra_flags=-Iinclude" .

# upload the code to the board
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:mega

# open the Serial Monitor at baudrate of 115200 for user input
arduino-cli monitor -p /dev/ttyACM0 --config baudrate=115200