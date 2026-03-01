#include "myheader.h"

// For timing fist hold reset
unsigned long fistHoldStart = 0;
bool motorActive = false;

void setup() {
  Serial.begin(115200);
  delay(1000);

  for (int i = 0; i < 5; i++) {
    motors[i].setMaxSpeed(MAX_STEPPER_SPEED);
    motors[i].setAcceleration(MAX_STEPPER_ACCEL);
  }

  myServo.attach(SERVO_PIN);
  myServo.write(servoPosition);

  Serial.println("Robotic Arm Ready");
  Serial.println("Waiting for Python signal to select motor (1-5)...");
}

void loop() {
  static String inputLine = "";
  static unsigned long fistHoldStart{};
  static bool motorActive{};
  char c{};
  int commaIndex{};
  int motor{};
  int cmd{};

  // ---------- SERIAL READING ----------
  while (Serial.available()) {
    c = Serial.read();
    if (c == '\n') {
      // parse line from python: <motor>,<command>
      commaIndex = inputLine.indexOf(',');
      if (commaIndex > 0) {
        motor = inputLine.substring(0, commaIndex).toInt();
        cmd   = inputLine.substring(commaIndex + 1).toInt();
        
        // ---------- MOTOR SELECTION ----------
        if (!motorActive && motor >= 1 && motor <= 5) {
          currentMode = motor;   // select motor once
          motorActive = true;
          Serial.print("Motor selected: ");
          Serial.println(currentMode);
        }

        // ---------- MOTOR CONTROL ----------
        if (motorActive && motor == currentMode) {
          switch (cmd) {
            case 1: // forward
              moveMotors(2);
              fistHoldStart = 0;
              break;
            case 2: // backward
              moveMotors(-2);
              fistHoldStart = 0;
              break;
            case 0: // stop/fist
              stopMotors();
              if (fistHoldStart == 0) fistHoldStart = millis();
              break;
            case 9: // idle/no detection
              // do nothing, just let motors run
              break;
          }
        }
      }
      inputLine = "";
    } else {
      inputLine += c;
    }
  }

  // -------- FIST HOLD RESET ----------
  if (fistHoldStart != 0 && millis() - fistHoldStart >= 5000) {
    currentMode = 0;
    motorActive = false;
    fistHoldStart = 0;
    Serial.println("Reset to motor selection screen");
    Serial.println("Waiting for Python signal to select motor (1-5)...");
  }

  // -------- RUN STEPPERS ----------
  for (int i = 0; i < 5; i++) {
    motors[i].run();
  }
}