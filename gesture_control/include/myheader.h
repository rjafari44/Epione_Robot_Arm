#pragma once

#include <AccelStepper.h>
#include <Servo.h>

// ---------------- PIN DEFINITIONS ----------------
constexpr int M1_STEP{44};
constexpr int M1_DIR{45};
constexpr int M2_STEP{42};
constexpr int M2_DIR{43};
constexpr int M3_STEP{40};
constexpr int M3_DIR{41};
constexpr int M4_STEP{38};
constexpr int M4_DIR{39};
constexpr int M5_STEP{34};
constexpr int M5_DIR{35};
constexpr int SERVO_PIN{6};

// ---------------- OBJECTS ----------------
inline Servo myServo;

inline AccelStepper motors[5] = {
  AccelStepper(AccelStepper::DRIVER, M1_STEP, M1_DIR), // 0
  AccelStepper(AccelStepper::DRIVER, M2_STEP, M2_DIR), // 1
  AccelStepper(AccelStepper::DRIVER, M3_STEP, M3_DIR), // 2
  AccelStepper(AccelStepper::DRIVER, M4_STEP, M4_DIR), // 3
  AccelStepper(AccelStepper::DRIVER, M5_STEP, M5_DIR)  // 4
};

constexpr int STEP_SCALE{60};
constexpr int MAX_STEPPER_SPEED{300};
constexpr int MAX_STEPPER_ACCEL{200};

// ---------------- GLOBAL VARIABLES ----------------
inline int currentMode{0};       // 0 = no motor selected
inline float servoPosition{90.0};

// ---------------- FUNCTION DECLARATIONS ----------------
void moveMotors(int dir);
void stopMotors();