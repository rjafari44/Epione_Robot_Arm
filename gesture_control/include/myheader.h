#pragma once

#include <AccelStepper.h>  // library for controlling stepper motors
#include <Servo.h>         // library for controlling servo motors

// ---------------- Pin Definitions ----------------
constexpr int M1_STEP{44};  // step pin for motor 1
constexpr int M1_DIR{45};   // direction pin for motor 1
constexpr int M2_STEP{42};  // step pin for motor 2
constexpr int M2_DIR{43};   // direction pin for motor 2
constexpr int M3_STEP{40};  // step pin for motor 3
constexpr int M3_DIR{41};   // direction pin for motor 3
constexpr int M4_STEP{38};  // step pin for motor 4
constexpr int M4_DIR{39};   // direction pin for motor 4
constexpr int M5_STEP{34};  // step pin for motor 5
constexpr int M5_DIR{35};   // direction pin for motor 5
constexpr int SERVO_PIN{6}; // pwm pin for the servo motor

// ---------------- Objects ----------------
inline Servo myServo; // servo object

inline AccelStepper motors[5] = {  // array of 5 stepper motors
  AccelStepper(AccelStepper::DRIVER, M1_STEP, M1_DIR), // Motor 1
  AccelStepper(AccelStepper::DRIVER, M2_STEP, M2_DIR), // Motor 2
  AccelStepper(AccelStepper::DRIVER, M3_STEP, M3_DIR), // Motor 3
  AccelStepper(AccelStepper::DRIVER, M4_STEP, M4_DIR), // Motor 4
  AccelStepper(AccelStepper::DRIVER, M5_STEP, M5_DIR)  // Motor 5
};

constexpr int STEP_SCALE{60};         // scaling factor for steps
constexpr int MAX_STEPPER_SPEED{300}; // max speed for stepper motors
constexpr int MAX_STEPPER_ACCEL{200}; // max acceleration for stepper motors

// ---------------- Global Variables ----------------
inline int currentMode{0};        // selected motor (0 = none)
inline float servoPosition{90.0}; // servo position in degrees

// ---------------- Function declarations ----------------
void moveMotors(int dir); // move selected motor(s) in direction
void stopMotors();        // stop all stepper motors