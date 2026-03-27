#include "myheader.h"

void setup() {
  Serial.begin(115200); // start serial communication at 115200 baud rate
  delay(1000);          // wait for serial to stabilize

  for (int i = 0; i < 6; i++) {  // configure all stepper motors
    motors[i].setMaxSpeed(MAX_STEPPER_SPEED);     // set max speed
    motors[i].setAcceleration(MAX_STEPPER_ACCEL); // set acceleration
  }

  motor7.attach(SERVO_PIN);      // attach servo motor to pwm pin
  motor7.write(servoPosition);   // set initial servo position
  pinMode(JOY_SW, INPUT_PULLUP); // configure joystick button as input with pull-up

  // print startup messages
  Serial.println("Robotic Arm Ready");
  Serial.println("Type 1-7 to select motor:");
  Serial.println("1=Servo  2=Opposite  3=Together  4=Base  5=LowerGearBox  6=UpperGearBox  7=Wrist");
}

void loop() {
  // ---------- Variables ----------
  static bool lastButtonState{HIGH};                // previous joystick button state
  static unsigned long lastServoUpdate{0};          // last time servo position was updated
  bool buttonState{};                               // current joystick button state
  int vx{}, direction{}, scaledStep{};              // joystick x-axis, direction, and step scaling
  unsigned long now{};                              // current time in ms
  float deltaTime{}, joystickPercent{}, velocity{}; // time diff, joystick fraction, servo velocity
  char input{};                                     // serial input character

  // ---------- Serial Motor Selection ----------
  if (Serial.available()) {
    input = Serial.read();              // read one character from serial
    if (input >= '1' && input <= '7') { // valid motor selection
      currentMode = input - '0';        // set current mode
      Serial.print("Selected Mode: ");     
      Serial.println(currentMode);
    } else if (input != '\n' && input != '\r') { // ignore newline or return
      Serial.print("Invalid input. Please enter a number between 1 and 7: ");
    }
  }

  // ---------- Joystick Button Cycling ----------
  buttonState = digitalRead(JOY_SW);                   // read joystick button
  if (buttonState == LOW && lastButtonState == HIGH) { // detect press event
    currentMode++;                                     // increment mode
    if (currentMode > 7) {
      currentMode = 1;              // wrap around after 7
    }
    Serial.print("Selected Mode: ");       
    Serial.println(currentMode);
    delay(200);                             // simple debounce
  }
  lastButtonState = buttonState;           // store last state

  // ---------- read joystick x-axis ----------
  vx = analogRead(JOY_VX) - 512;           // center joystick around 0
  if (abs(vx) < DEAD_ZONE) vx = 0;         // apply dead zone

  // ---------- servo control (mode 1) ----------
  if (currentMode == 1) {
    now = millis();                               // get current time
    deltaTime = (now - lastServoUpdate) / 1000.0; // compute elapsed time in seconds
    lastServoUpdate = now;                        // update last servo time

    if (abs(vx) > SERVO_DEAD_ZONE) {                // only move if outside dead zone
      joystickPercent = (float)vx / 512.0;          // fraction of full joystick deflection
      velocity = joystickPercent * SERVO_MAX_SPEED; // desired speed
      servoPosition += velocity * deltaTime;        // update servo position
    }
    servoPosition = constrain(servoPosition, 0, 180); // clamp to valid range
    motor7.write(servoPosition);                      // write to servo
  }

  // ---------- Stepper Control (modes 2-7) --------- 
  if (vx != 0 && currentMode != 1) {
    if (vx > 0) { // determine movement direction based on joystick x-axis
      direction = 1;  // joystick pushed right → move forward
    } else {
      direction = -1; // joystick pushed left → move backward
    }

  scaledStep = map(abs(vx), 0, 512, 0, STEP_SCALE); // scale joystick magnitude to stepper step size
  moveMotors(direction); // move selected motor(s) using support cpp function
}

  // ---------- run all steppers ----------
  for (int i = 0; i < 6; i++) motors[i].run(); // update stepper positions
}