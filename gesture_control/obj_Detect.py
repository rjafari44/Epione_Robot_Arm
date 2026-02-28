import cv2
import mediapipe as mp
import time
from collections import deque
import serial

print("Starting Industrial Gesture Motor Control (Continuous Feedback with Selection Delay)...")

# ---------------- Serial Setup ----------------
ser = serial.Serial('/dev/ttyACM0', 115200, timeout=0.1)  # change port if needed
time.sleep(2)  # wait for Arduino to reset

# ---------------- Mediapipe Setup ----------------
mp_hands = mp.solutions.hands
hands = mp_hands.Hands(max_num_hands=1)
mp_draw = mp.solutions.drawing_utils

cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("ERROR: Could not open camera.")
    exit()

print("Camera started.")
print("Hold FIST for 5 seconds to enter SELECTION MODE.\n")

# ---------------- States ----------------
STATE_IDLE = 0
STATE_SELECTION = 1
STATE_DIRECTION = 2

state = STATE_IDLE
selected_motor = None
selection_time = None  # time when motor was selected

# ---------------- Settings ----------------
STABLE_FRAMES_REQUIRED = 8
TRIGGER_HOLD_TIME = 5.0  # seconds
SELECTION_DELAY = 2.0    # seconds before motor reacts
CONSOLE_UPDATE_INTERVAL = 0.5  # seconds

gesture_buffer = deque(maxlen=STABLE_FRAMES_REQUIRED)
current_stable_gesture = None
gesture_hold_start = None
last_console_update = 0

# ---------------- Finger Detection ----------------
def get_finger_states(hand_landmarks):
    finger_tips = [4, 8, 12, 16, 20]
    finger_dips = [3, 6, 10, 14, 18]
    fingers = {}

    fingers["thumb"] = hand_landmarks.landmark[4].x < hand_landmarks.landmark[3].x
    names = ["index", "middle", "ring", "pinky"]

    for i, tip in enumerate(finger_tips[1:]):
        fingers[names[i]] = hand_landmarks.landmark[tip].y < hand_landmarks.landmark[finger_dips[i + 1]].y

    return fingers

def count_extended(finger_states):
    return sum(finger_states.values())

def get_stable_gesture(new_value):
    gesture_buffer.append(new_value)
    if len(gesture_buffer) < STABLE_FRAMES_REQUIRED:
        return None
    if all(v == gesture_buffer[0] for v in gesture_buffer):
        return gesture_buffer[0]
    return None

# ---------------- Main Loop ----------------
while True:
    success, img = cap.read()
    if not success:
        break

    img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    results = hands.process(img_rgb)
    stable_gesture = None

    if results.multi_hand_landmarks:
        handLms = results.multi_hand_landmarks[0]
        mp_draw.draw_landmarks(img, handLms, mp_hands.HAND_CONNECTIONS)

        finger_states = get_finger_states(handLms)
        finger_count = count_extended(finger_states)
        stable_gesture = get_stable_gesture(finger_count)
    else:
        gesture_buffer.clear()
        stable_gesture = None
        current_stable_gesture = None
        gesture_hold_start = None

    current_time = time.time()

    # ---------------- Handle Gesture Changes ----------------
    if stable_gesture is not None:
        if stable_gesture != current_stable_gesture:
            current_stable_gesture = stable_gesture
            gesture_hold_start = current_time
            trigger_fired = False

        held_time = current_time - gesture_hold_start if gesture_hold_start else 0

        # ---------------- STATE MACHINE ----------------
        if stable_gesture == 0 and held_time >= TRIGGER_HOLD_TIME:
            # Fist triggers selection mode from idle or direction
            if state != STATE_SELECTION:
                state = STATE_SELECTION
                selected_motor = None
                print("\n>>> ENTERED SELECTION MODE <<<\n")
        elif state == STATE_SELECTION:
            if 1 <= stable_gesture <= 5 and held_time >= TRIGGER_HOLD_TIME:
                selected_motor = stable_gesture
                state = STATE_DIRECTION
                selection_time = current_time  # mark selection time
                print(f"\n>>> MOTOR {selected_motor} SELECTED <<<\n")

    # ---------------- Continuous Console Updates ----------------
    if state == STATE_SELECTION:
        if current_stable_gesture and 1 <= current_stable_gesture <= 5:
            print(f"Holding Motor {current_stable_gesture}: {held_time:.1f}s", end="\r")

    if state == STATE_DIRECTION and selected_motor is not None:
        # Only allow motor movement after SELECTION_DELAY
        if selection_time and current_time - selection_time < SELECTION_DELAY:
            print(f"Motor {selected_motor} → WAITING ({SELECTION_DELAY - (current_time - selection_time):.1f}s)", end="\r")
            gesture_command = 9  # IDLE while waiting
        else:
            if stable_gesture == 1:
                gesture_command = 1  # FORWARD
            elif stable_gesture == 2:
                gesture_command = 2  # BACKWARD
            elif stable_gesture == 0:
                gesture_command = 0  # STOP
            else:
                gesture_command = 9  # IDLE

            # Send command to Arduino
            ser.write(f"{selected_motor},{gesture_command}\n".encode())

            # Keep console feedback exactly the same
            if stable_gesture == 1 and current_time - last_console_update > CONSOLE_UPDATE_INTERVAL:
                print(f"Motor {selected_motor} → FORWARD")
                last_console_update = current_time
            elif stable_gesture == 2 and current_time - last_console_update > CONSOLE_UPDATE_INTERVAL:
                print(f"Motor {selected_motor} → BACKWARD")
                last_console_update = current_time
            elif stable_gesture == 0 and current_time - last_console_update > CONSOLE_UPDATE_INTERVAL:
                print(f"Motor {selected_motor} → STOP")
                last_console_update = current_time
            elif current_time - last_console_update > CONSOLE_UPDATE_INTERVAL:
                print(f"Motor {selected_motor} → IDLE")
                last_console_update = current_time

    cv2.imshow("Industrial Hand Tracking", img)
    if cv2.waitKey(1) & 0xFF == 27:
        break

cap.release()
cv2.destroyAllWindows()
ser.close()
print("Program closed cleanly.")