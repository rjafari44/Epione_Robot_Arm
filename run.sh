#!/bin/bash

# ask the user for their Arduino Mega board
read -p "Enter your Arduino Mega Port (e.g., /dev/ttyACM0): " PORT

# compile the code with 
arduino-cli compile --fqbn arduino:avr:mega --build-property "compiler.cpp.extra_flags=-Iinclude" .

# upload the code to the board
arduino-cli upload -p $PORT --fqbn arduino:avr:mega

# open the Serial Monitor at baudrate of 115200 for user input
arduino-cli monitor -p $PORT --config baudrate=115200