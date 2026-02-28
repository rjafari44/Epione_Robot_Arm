#include "myheader.h"

// some global variables
constexpr int MAX_STEPPER_SPEED{1000};
constexpr int MAX_STEPPER_ACCEL{800};
constexpr int STEP_SCALE{60};
constexpr int DEAD_ZONE{40};

// servo variables
constexpr float SERVO_MAX_SPEED{60.0}; // in degrees per second
constexpr int SERVO_DEAD_ZONE{60};
float servoPosition{90.0};

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  for (int i = 0; i < 6; i++) {
    motors[i].setMaxSpeed(MAX_STEPPER_SPEED);
    motors[i].setAcceleration(MAX_STEPPER_ACCEL);
  }

  motor7.attach(SERVO_PIN);
  motor7.write(servoPosition);

  pinMode(JOY_SW, INPUT_PULLUP);

  Serial.println("Robotic Arm Ready");
  Serial.println("Type 1-7 to select motor:");
  Serial.println("1=Servo  2=Opposite  3=Together  4=Base  5=LowerGearBox  6=UpperGearBox  7=Wrist");
}

void loop() {
  // -------- VARIABLES --------
  bool buttonState{};
  int vx{};
  int direction{};
  int scaledStep{};
  unsigned long now{};
  float deltaTime{};
  float joystickPercent{};
  float velocity{};
  char input{};
  static int currentMode{1};   // default mode, gripper
  static bool lastButtonState{HIGH};
  static unsigned long lastServoUpdate{0};

  // -------- SERIAL SELECTION --------
  if (Serial.available()) {
    input = Serial.read();

    if (input >= '1' && input <= '7') {
      currentMode = input - '0';
      Serial.print("Selected Mode: ");
      Serial.println(currentMode);
    } else if (input != '\n' && input != '\r') { // ignore newline/return
        Serial.print("Invalid input. Please enter a number between 1 and 7: ");
    }
  }

  // joystick button for optional cycling
  buttonState = digitalRead(JOY_SW);
  if (buttonState == LOW && lastButtonState == HIGH) {
    currentMode++;

    if (currentMode > 7) {
      currentMode = 1;
    }
    Serial.print("Selected Mode: ");
    Serial.println(currentMode);
    delay(200);
  }
  lastButtonState = buttonState;

  vx = analogRead(JOY_VX) - 512;
  if (abs(vx) < DEAD_ZONE) {
    vx = 0;
  }

    // servo (case 1)
  if (currentMode == 1) {
    now = millis();
    deltaTime = (now - lastServoUpdate) / 1000.0;
    lastServoUpdate = now;

    if (abs(vx) > SERVO_DEAD_ZONE) {
      joystickPercent = (float)vx / 512.0;
      velocity = joystickPercent * SERVO_MAX_SPEED;
      servoPosition += velocity * deltaTime;
    }

    servoPosition = constrain(servoPosition, 0, 180);
    motor7.write(servoPosition);
  }

  // stepper control
  if (vx != 0 && currentMode != 1) {
    if (vx > 0) {
      direction = 1;
    } else {
      direction = -1;
    }
    scaledStep = map(abs(vx), 0, 512, 0, STEP_SCALE);

    switch (currentMode) {
      case 2: // opposite
        motors[0].moveTo(motors[0].currentPosition() + direction * scaledStep);
        motors[3].moveTo(motors[3].currentPosition() + direction * scaledStep);
        break;
      case 3: // together
        motors[0].moveTo(motors[0].currentPosition() + direction * scaledStep);
        motors[3].moveTo(motors[3].currentPosition() - direction * scaledStep);
        break;
      case 4: // base motor
        motors[5].moveTo(motors[5].currentPosition() + direction * scaledStep);
        break;
      case 5: // Lower GearBox
        motors[1].moveTo(motors[1].currentPosition() + direction * scaledStep);
        break;
      case 6: // upper GearBox
        motors[2].moveTo(motors[2].currentPosition() + direction * scaledStep);
        break;
      case 7: // Wrist
        motors[4].moveTo(motors[4].currentPosition() + direction * scaledStep);
        break;
    }
  }

  // -------- RUN STEPPERS --------
  for (int i = 0; i < 6; i++) {
    motors[i].run();
  }
}