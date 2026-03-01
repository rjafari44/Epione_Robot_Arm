#include "myheader.h"

// moving the motors, taking a set servo speed as input
void moveMotors(int dir) {
  int scaledStep{};               // scaled step amount for steppers
  scaledStep = STEP_SCALE * dir;  // calculate scaled step

  switch (currentMode) {
    case 1: // servo
      servoPosition += dir;          // update servo position
      servoPosition = constrain(servoPosition, 0, 180); // keep servo within 0-180
      myServo.write(servoPosition);  // move servo
      break;
    case 2: // together (motors 1+4)
      motors[0].moveTo(motors[0].currentPosition() + scaledStep); // move motor 1
      motors[3].moveTo(motors[3].currentPosition() - scaledStep); // move motor 4 opposite (due to wiring)
      break;
    case 3: // base motor
      motors[4].setMaxSpeed(600);     // set speed for motor 5
      motors[4].setAcceleration(400); // set acceleration for motor 5
      motors[4].moveTo(motors[4].currentPosition() + scaledStep); // move motor 5
      break;
    case 4: // lower gearbox
      motors[1].moveTo(motors[1].currentPosition() + scaledStep); // move motor 2
      break;
    case 5: // upper gearbox
      motors[2].moveTo(motors[2].currentPosition() + scaledStep); // move motor 3
      break;
    default:
      break; // do nothing if no motor selected
  }
}

// stop motors
void stopMotors() {
  if (currentMode == 1) {
    myServo.write(servoPosition); // hold servo in place
  } else {
    for (int i = 0; i < 5; i++) {
      motors[i].stop(); // stop each stepper motor
    }
  }
}