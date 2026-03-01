#include "myheader.h"

void setup() {
  Serial.begin(115200); // initialize serial communication at 115200 baud rate
  delay(1000);          // wait for serial to stabilize

  for (int i = 0; i < 5; i++) {
    motors[i].setMaxSpeed(MAX_STEPPER_SPEED);     // set max speed for each motor
    motors[i].setAcceleration(MAX_STEPPER_ACCEL); // set acceleration for each motor
  }

  myServo.attach(SERVO_PIN);    // attach servo to its pin
  myServo.write(servoPosition); // set initial servo position

  Serial.println("Robotic Arm Ready"); // notify ready state
  Serial.println("Waiting for Python signal to select motor (1-5)..."); // prompt motor selection
}

void loop() {
  // ---------- Variables ----------
  static String inputLine = "";         // buffer for incoming serial data
  static unsigned long fistHoldStart{}; // timestamp for stop/fist hold
  static bool motorActive{};            // track if motor is currently active
  char c{};                             // current serial character
  int commaIndex{}, motor{}, cmd{};     // index of comma in input, motor number from input, command from input

  // ---------- Serial Reading ----------
  while (Serial.available()) {
    c = Serial.read();                     // read a character
    if (c == '\n') {                       // end of line received
      commaIndex = inputLine.indexOf(','); // find comma separator
      if (commaIndex > 0) {
        motor = inputLine.substring(0, commaIndex).toInt(); // parse motor number
        cmd   = inputLine.substring(commaIndex + 1).toInt(); // parse command
        
        // ---------- Motor Selection ----------
        if (!motorActive && motor >= 1 && motor <= 5) {
          currentMode = motor;              // select motor once
          motorActive = true;               // mark motor as active
          Serial.print("Motor selected: "); // notify selection
          Serial.println(currentMode);
        }

        // ---------- Motor Control ----------
        if (motorActive && motor == currentMode) {
          switch (cmd) {
            case 1: // forward
              moveMotors(2);     // move motor forward
              fistHoldStart = 0; // reset fist hold timer
              break;
            case 2: // backward
              moveMotors(-2);    // move motor backward
              fistHoldStart = 0; // reset fist hold timer
              break;
            case 0: // stop/fist
              stopMotors();      // stop motor
              if (fistHoldStart == 0) fistHoldStart = millis(); // start hold timer
              break;
            case 9: // idle/no detection
              break;             // do nothing
          }
        }
      }
      inputLine = ""; // clear input buffer
    } else {
      inputLine += c; // append char to input buffer
    }
  }

  // -------- Fist Hold Reset ----------
  if (fistHoldStart != 0 && millis() - fistHoldStart >= 5000) {
    currentMode = 0;     // reset selected motor
    motorActive = false; // mark motor as inactive
    fistHoldStart = 0;   // reset hold timer
    Serial.println("Reset to motor selection screen"); // notify reset
    Serial.println("Waiting for Python signal to select motor (1-5)..."); // prompt selection
  }

  // -------- Run Steppers ----------
  for (int i = 0; i < 5; i++) {
    motors[i].run(); // step each motor
  }
}