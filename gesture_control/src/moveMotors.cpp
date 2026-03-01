#include "myheader.h"

void moveMotors(int dir) {
  int scaledStep{};
  
  scaledStep = STEP_SCALE* dir;

  switch (currentMode) {
    case 1: // Servo
      servoPosition += dir;
      servoPosition = constrain(servoPosition, 0, 180);
      myServo.write(servoPosition);
      break;
    case 2: // together (motors 0+3)
      motors[0].moveTo(motors[0].currentPosition() + scaledStep);
      motors[3].moveTo(motors[3].currentPosition() - scaledStep);
      break;
    case 3: // base motor
      motors[4].moveTo(motors[4].currentPosition() + scaledStep);
      break;
    case 4: // Lower GearBox
      motors[1].moveTo(motors[1].currentPosition() + scaledStep);
      break;
    case 5: // Upper GearBox
      motors[2].moveTo(motors[2].currentPosition() + scaledStep);
      break;
    default:
      break;
  }
}

void stopMotors() {
  if (currentMode == 1) {
    myServo.write(servoPosition); // hold servo
  } else {
    for (int i = 0; i < 5; i++) {
      motors[i].stop(); // stop steppers
    }
  }
}