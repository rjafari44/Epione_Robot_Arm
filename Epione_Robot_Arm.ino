#include <AccelStepper.h>
#include <Servo.h>

// ---------------- PIN DEFINITIONS ----------------
#define M1_STEP 44
#define M1_DIR  45
#define M2_STEP 42
#define M2_DIR  43
#define M3_STEP 40
#define M3_DIR  41
#define M4_STEP 38
#define M4_DIR  39
#define M5_STEP 36
#define M5_DIR  37
#define M6_STEP 34
#define M6_DIR  35

#define SERVO_PIN 6

#define JOY_VX A2
#define JOY_SW 7

// ---------------- OBJECTS ----------------
AccelStepper motors[6] = {
  AccelStepper(AccelStepper::DRIVER, M1_STEP, M1_DIR), // 0
  AccelStepper(AccelStepper::DRIVER, M2_STEP, M2_DIR), // 1
  AccelStepper(AccelStepper::DRIVER, M3_STEP, M3_DIR), // 2
  AccelStepper(AccelStepper::DRIVER, M4_STEP, M4_DIR), // 3
  AccelStepper(AccelStepper::DRIVER, M5_STEP, M5_DIR), // 4
  AccelStepper(AccelStepper::DRIVER, M6_STEP, M6_DIR)  // 5
};

Servo motor7;

// ---------------- VARIABLES ----------------
const int maxStepperSpeed = 1000;
const int maxStepperAccel = 800;
const int stepScale = 60;
const int deadzone = 40;

int currentMode = 3;   // default: opposite
bool lastButtonState = HIGH;

// ---- Servo Control ----
float servoPosition = 90.0;
unsigned long lastServoUpdate = 0;
const float servoMaxSpeedDegPerSec = 60.0;
const int servoDeadzone = 60;

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 6; i++) {
    motors[i].setMaxSpeed(maxStepperSpeed);
    motors[i].setAcceleration(maxStepperAccel);
  }

  // Invert M4 if needed
  // motors[3].setPinsInverted(true, false, false);

  motor7.attach(SERVO_PIN);
  motor7.write(servoPosition);

  pinMode(JOY_SW, INPUT_PULLUP);

  Serial.println("Robotic Arm Ready");
  Serial.println("Type 1-7 to select motor:");
  Serial.println("1=Servo  2=Together  3=Opposite  4=M2  5=M3  6=M5  7=M6");
}

// ---------------- LOOP ----------------
void loop() {

  // -------- SERIAL SELECTION --------
  if (Serial.available()) {
    char input = Serial.read();

    if (input >= '1' && input <= '7') {
      currentMode = input - '0';
      Serial.print("Selected Mode: ");
      Serial.println(currentMode);
    }
  }

  // -------- JOYSTICK BUTTON (OPTIONAL CYCLING) --------
  bool buttonState = digitalRead(JOY_SW);
  if (buttonState == LOW && lastButtonState == HIGH) {
    currentMode++;
    if (currentMode > 7) currentMode = 1;
    Serial.print("Selected Mode: ");
    Serial.println(currentMode);
    delay(200);
  }
  lastButtonState = buttonState;

  int vx = analogRead(JOY_VX) - 512;
  if (abs(vx) < deadzone) vx = 0;

  // -------- STEPPER CONTROL --------
  if (vx != 0 && currentMode != 1) {

    int direction = (vx > 0) ? 1 : -1;
    int scaledStep = map(abs(vx), 0, 512, 0, stepScale);

    switch (currentMode) {

      case 2: // Together
        motors[0].moveTo(motors[0].currentPosition() + direction * scaledStep);
        motors[3].moveTo(motors[3].currentPosition() + direction * scaledStep);
        break;

      case 3: // Opposite
        motors[0].moveTo(motors[0].currentPosition() + direction * scaledStep);
        motors[3].moveTo(motors[3].currentPosition() - direction * scaledStep);
        break;

      case 4: // M2
        motors[1].moveTo(motors[1].currentPosition() + direction * scaledStep);
        break;

      case 5: // M3
        motors[2].moveTo(motors[2].currentPosition() + direction * scaledStep);
        break;

      case 6: // M5
        motors[4].moveTo(motors[4].currentPosition() + direction * scaledStep);
        break;

      case 7: // M6
        motors[5].moveTo(motors[5].currentPosition() + direction * scaledStep);
        break;
    }
  }

  // -------- SERVO CONTROL --------
  if (currentMode == 1) {

    unsigned long now = millis();
    float deltaTime = (now - lastServoUpdate) / 1000.0;
    lastServoUpdate = now;

    if (abs(vx) > servoDeadzone) {
      float joystickPercent = (float)vx / 512.0;
      float velocity = joystickPercent * servoMaxSpeedDegPerSec;
      servoPosition += velocity * deltaTime;
    }

    servoPosition = constrain(servoPosition, 0, 180);
    motor7.write(servoPosition);
  }

  // -------- RUN STEPPERS --------
  for (int i = 0; i < 6; i++) {
    motors[i].run();
  }
}