#include "myheader.h"

// move the selected motor(s) or servo based on current mode and direction
void moveMotors(int dir) {
  int scaledStep{};                         
  scaledStep = STEP_SCALE * dir; // calculate scaled step based on direction

  switch (currentMode) {
    case 1: // servo control
      servoPosition += dir;                   // update servo position
      servoPosition = constrain(servoPosition, 0, 180); // limit to 0-180 deg
      motor7.write(servoPosition);           // write new position to servo
      break;
    case 2: // opposite movement (motors 0 and 3)
      motors[0].moveTo(motors[0].currentPosition() + scaledStep); // move motor 1
      motors[3].moveTo(motors[3].currentPosition() + scaledStep); // move motor 4
      break;
    case 3: // together movement (motors 0 and 3 in opposite directions)
      motors[0].moveTo(motors[0].currentPosition() + scaledStep); // move motor 1 forward
      motors[3].moveTo(motors[3].currentPosition() - scaledStep); // move motor 4 backward
      break;
    case 4: // base motor
      motors[5].setMaxSpeed(400);     // set speed for motor 6
      motors[5].setAcceleration(200); // set acceleration for motor 6
      motors[5].moveTo(motors[5].currentPosition() + scaledStep); // move motor 6
      break;
    case 5: // lower gearbox
      motors[1].moveTo(motors[1].currentPosition() + scaledStep); // move motor 2
      break;
    case 6: // upper gearbox
      motors[2].moveTo(motors[2].currentPosition() + scaledStep); // move motor 3
      break;
    case 7: // wrist
      motors[4].moveTo(motors[4].currentPosition() + scaledStep); // move motor 5
      break;
    default: // no motor selected
      break;
  }
}

// stop all stepper motors or hold servo in place
void stopMotors() {
  if (currentMode == 1) {
    motor7.write(servoPosition); // hold servo at current position
  } else {
    for (int i = 0; i < 6; i++) motors[i].stop(); // stop each stepper motor
  }
}