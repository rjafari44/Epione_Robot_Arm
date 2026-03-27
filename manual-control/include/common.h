#pragma once

#include <AccelStepper.h>  // library for stepper motors
#include <Servo.h>         // library for servo motor

// ---------------- Pin Definitions ----------------
constexpr int M1_STEP{44};  // step pin for motor 1
constexpr int M1_DIR{45};   // direction pin for motor 1
constexpr int M2_STEP{42};  // step pin for motor 2
constexpr int M2_DIR{43};   // direction pin for motor 2
constexpr int M3_STEP{40};  // step pin for motor 3
constexpr int M3_DIR{41};   // direction pin for motor 3
constexpr int M4_STEP{38};  // step pin for motor 4
constexpr int M4_DIR{39};   // direction pin for motor 4
constexpr int M5_STEP{36};  // step pin for motor 5
constexpr int M5_DIR{37};   // direction pin for motor 5
constexpr int M6_STEP{34};  // step pin for motor 6
constexpr int M6_DIR{35};   // direction pin for motor 6
constexpr int SERVO_PIN{6}; // pwm pin for servo motor
constexpr int JOY_VX{A2};   // analog x-axis of joystick
constexpr int JOY_SW{7};    // joystick button pin

inline Servo motor7;         // servo object for motor 7

// ---------------- Objects ----------------
inline AccelStepper motors[6] = {  // array of 6 stepper motors
  AccelStepper(AccelStepper::DRIVER, M1_STEP, M1_DIR), // motor 1
  AccelStepper(AccelStepper::DRIVER, M2_STEP, M2_DIR), // motor 2
  AccelStepper(AccelStepper::DRIVER, M3_STEP, M3_DIR), // motor 3
  AccelStepper(AccelStepper::DRIVER, M4_STEP, M4_DIR), // motor 4
  AccelStepper(AccelStepper::DRIVER, M5_STEP, M5_DIR), // motor 5
  AccelStepper(AccelStepper::DRIVER, M6_STEP, M6_DIR)  // motor 6
};

// ---------------- Global Variables ----------------
constexpr int MAX_STEPPER_SPEED{1000}; // max speed for steppers
constexpr int MAX_STEPPER_ACCEL{800};  // max acceleration for steppers
constexpr int STEP_SCALE{60};          // step scaling factor
constexpr int DEAD_ZONE{40};           // joystick dead zone threshold
inline int currentMode{1};

// ---------------- Servo Variables ----------------
constexpr float SERVO_MAX_SPEED{60.0}; // max servo speed in deg/sec
constexpr int SERVO_DEAD_ZONE{60};     // servo dead zone threshold
inline float servoPosition{90.0};             // current servo position

// ---------------- Function Declarations ----------------
void moveMotors(int dir);  // move selected motor(s) or servo in given direction
void stopMotors();         // stop all steppers or hold servo in place