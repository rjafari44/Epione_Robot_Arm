# Robot Arm

An ARCTOS-based robotic arm featuring a custom end effector, custom wiring architecture, and dual-mode control software. This project was completed in collaboration with a group of students at my university, and it would not have been possible without their collective effort.

The system is divided into four primary sections:

- [Mechanical](#mechanical)
- [Electrical](#electrical)
- [Programming](#programming)
- [How to Run](#how-to-run)


A Bill of Materials (BOM) is not included here, as the official ARCTOS BOM covers the majority of required components. This repository contains only files and documentation original to our group, including custom CAD files and wiring schematics.

Image of robot arm:  
*(Insert image here)*

---

## Mechanical

All structural components were 3D printed using PLA+.

### Print Settings

- Infill:  
  - 25% for structural components  
  - 30–35% for small gears  
- Layer height: 0.15 mm  
- Horizontal shells:  
  - Top layers: 7  
  - Bottom layers: 5  
  - Minimum top thickness: 0.7 mm  
  - Minimum bottom thickness: 0.5 mm  
- Vertical shells: 2 perimeters  
- Supports: Enabled where necessary for overhangs  
- Brim: Used for gears  

Temperature settings were filament- and printer-specific and are not documented here.

The CAD files for the robot body were purchased as part of the ARCTOS kit and are not redistributed.

### Custom End Effector

We designed a custom linkage-type end effector actuated by a single 25 kg servo motor. The gripping force is moderate but sufficient for our application, as the robot arm is rated for approximately 1 kg of payload.

The design prioritizes:

- Mechanical simplicity  
- Direct servo actuation  
- Easy maintenance  

All custom end effector CAD files are included in this repository.

### Assembly Notes

Assembly was significantly more involved than expected. The provided instructions were poorly formatted, skipped steps, and did not clearly illustrate certain assembly procedures. As a result, we relied heavily on:

- The CAD assembly STEP file  
- Online video references  

We chose not to install aesthetic cover plates in order to maintain continuous access to wiring for adjustments and troubleshooting.

Lubricant was applied extensively to gear interfaces to prevent binding and seizing.

---

## Electrical

The electrical system was implemented with flexibility in mind. Specific pin mappings are not enforced, as different builds may require different configurations.

### Core Components

- Microcontroller: Arduino Mega  
- Motor control: Two CNC motor shields  
- Drivers: TMC2209 stepper motor drivers  
- Stepper motor voltage: 24V  
- Servo motor voltage: 4.8–8.4V  
- Buck converter: Used for servo voltage regulation  

The two CNC shields provide a total of eight motor ports. Only six were used, corresponding to the six stepper motors in the system. The servo motor was powered separately through the buck converter.

The kit included two cooling fans; only one was installed in the main electronics housing at the base. Additional cooling was deemed unnecessary.

### Limit Switches

The original ARCTOS design includes magnetically activated limit switches. These were intentionally omitted due to:

- Wiring complexity  
- Observed unreliability  
- Increased assembly overhead  

Instead, we rely on:

- Slow motor operation  
- Manual visual checks 

This is an open-loop configuration. No closed-loop feedback or encoder-based position verification is used.

The official ARCTOS electrical schematic is available on the manufacturer’s website. The exact wiring configuration used in this build is attached below. I have simplified the model of the wiring diagram from the website, including only necessary pins.

Circuit schematic:  
![Diagram](/assets/Schematic.png)

*Note: the schematic of the gesture mode is identical. No need to disconnect the joystick, it just won't get used.*

**Important:**  
When uploading firmware, ensure that motor power is disconnected. Leaving motor power enabled during upload may cause unintended jittering.

---

## Programming

The software implementation introduces two independent control modes:

1. Manual joystick control  
2. Vision-based object detection control using MediaPipe  

---

### Manual Mode

Manual mode uses a joystick with three inputs:

- Vx  
- Vy  
- Sw  

Only Vx and Sw are used in this implementation.

- Vx controls forward and reverse movement of the currently selected motor.  
- Sw cycles through available motors.  

Serial Monitor input is also supported and recommended for clarity, as it displays which motor is currently selected.

#### Manual Mode Control Summary

| Input | Function |
|--------|----------|
| Vx (positive) | Move selected motor forward |
| Vx (negative) | Move selected motor backward |
| Sw | Cycle to next motor |
| Serial input | Manual motor selection / debugging |

This mode is primarily used for calibration, testing, and precise positioning.

---

### Vision Mode (MediaPipe Gesture Control)

The second control mode uses Python and MediaPipe to detect hand gestures and translate them into motor commands. Please use the console to double check what is happening. Also, we used the AccelStepper and Servo Arduino libraries for the motor controls. Also two of the motors are not used for ease of use with 5 fingers.

Important constraints:

- It is single handed 
- The back of the hand must face the camera for stability
- The palm should face the user  
- Left-hand detection was found to be somewhat unstable  

The interaction flow is state-based.

#### Gesture Logic

1. Hold a fist for 5 seconds to enter **Selection Mode**.  
2. In Selection Mode, hold up 1–5 fingers for 5 seconds to select a motor.  
3. After selection, a 2-second countdown begins.  
4. Once active:
   - 1 finger → Move motor forward  
   - 2 fingers → Move motor backward  
   - Fist → Stop motor  
   - Any other gesture (or lack of detection) → Idle  
5. Holding a fist for 5 seconds again re-enters Selection Mode.

#### Vision Mode Control Table

| Gesture | Duration | Action |
|----------|----------|--------|
| Fist | 5 seconds | Enter selection mode |
| 1–5 fingers | 5 seconds | Select corresponding motor |
| 1 finger | Continuous | Move selected motor forward |
| 2 fingers | Continuous | Move selected motor backward |
| Fist (during control) | <5 Seconds | Stop motor |
| Fist (5 seconds anytime) | 5 seconds | Return to selection mode |

Python 3.10.x is required. A text file containing the required Python libraries is included in this repository.

---

## How to Run

I ran this code on an **Arduino Mega** using **arduino-cli** on Linux.  

Since this project uses both **Python** and **Arduino CLI** on Linux, I do not have documentation for other operating systems. To simplify compilation and upload, I have created shell scripts. Running these scripts from the CLI will automatically compile and upload the firmware to `/dev/ttyACM0`. If your board uses a different device port, you will need to manually update the Python code and the shell scripts to reflect the correct port. Also, each of the scripts are independent of each other, for the gesture control, upload the arduino code first then run the Python Script

**Important Notes:**

- If you are using **gesture control**, make sure to **exit the Serial Monitor** after uploading the firmware. The Python script needs exclusive access to the serial port. The Serial Monitor is only enabled by default for debugging.  
- **CDC on boot** is enabled by default.  
- If you prefer manual compilation and upload, you can either edit the shell scripts or copy the commands one at a time with custom flags.

---

### 1. Find Connected Serial Port (to double check)

#### On Linux (generic method)
```bash
ls /dev/tty*
```

#### With Arduino CLI (recommended method)
```bash
arduino-cli board list
```

---

### 2. Run the manual program in CLI
```bash
./runManual.sh
```

---

### 3. Run the gesture-control program in CLI
```bash
./runGesture.sh
```
#### exit then
```bash
python obj_Detect.py
```


---

<br>

## Acknowledgment

This project was completed as a collaborative university effort. Significant credit is due to all team members involved in mechanical assembly, electrical integration, programming, and testing.  

Below is the team and their LinkedIn profiles for reference:

| Name | Role | LinkedIn |
|------|------|---------|
| Radin Jafari | Electrical & Programming | [LinkedIn](https://www.linkedin.com/in/radin-jafari/) |
| Wilson Yang | Mechanical & Design | [LinkedIn](https://www.linkedin.com/in/shihweiyang/) |
| Hongyi Zhu | Mechanical & Assembly | [LinkedIn](https://www.linkedin.com/in/hongyi-zhu-065a71336/) |
| Brady Dong | Mechanical & Assembly | [LinkedIn](https://www.linkedin.com/in/brady-dong/) |
