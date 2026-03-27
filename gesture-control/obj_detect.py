import cv2
import mediapipe as mp
import time
from collections import deque
import serial

print("Starting Industrial Gesture Motor Control (Continuous Feedback with Selection Delay)...")

# ---------------- Serial Setup ----------------
ser = serial.Serial('/dev/ttyACM0', 115200, timeout=0.1)  # open serial port for Arduino with baudrate 115200
time.sleep(2)  # wait 2s to allow Arduino to reset and be ready

# ---------------- MediaPipeSetup ----------------
mp_hands = mp.solutions.hands  # load Mediapipe hand tracking module
hands = mp_hands.Hands(max_num_hands=1)  # track max 1 hand per frame
mp_draw = mp.solutions.drawing_utils  # utilities to draw hand landmarks

cap = cv2.VideoCapture(0)  # start webcam capture at default camera index
if not cap.isOpened():
    print("ERROR: Could not open camera.")
    exit()  # exit if camera fails to start

print("Camera started.")
print("Hold FIST for 5 seconds to enter SELECTION MODE.\n")

# ---------------- States ----------------
STATE_IDLE = 0  # no motor selected, waiting for user input
STATE_SELECTION = 1  # user is selecting which motor to control
STATE_DIRECTION = 2  # motor selected, waiting for gesture command

state = STATE_IDLE  # initialize state machine in idle
selected_motor = None  # variable to store which motor is selected
selection_time = None  # timestamp when motor selection occurs

# ---------------- Settings ----------------
STABLE_FRAMES_REQUIRED = 8  # number of consecutive frames to confirm a gesture
TRIGGER_HOLD_TIME = 5.0  # duration to hold a gesture before triggering action
SELECTION_DELAY = 2.0    # delay after motor selection before movement command is sent
CONSOLE_UPDATE_INTERVAL = 0.5  # interval in seconds for console output refresh

gesture_buffer = deque(maxlen=STABLE_FRAMES_REQUIRED)  # store last N finger counts to detect stable gesture
current_stable_gesture = None  # last confirmed stable gesture
gesture_hold_start = None  # timestamp when current stable gesture started
last_console_update = 0  # timestamp of last console print for rate-limiting output

# ---------------- Finger Detection ----------------
def get_finger_states(hand_landmarks):
    finger_tips = [4, 8, 12, 16, 20]  # Mediapipe landmark indices for fingertips
    finger_dips = [3, 6, 10, 14, 18]  # landmarks just below fingertips
    fingers = {}

    fingers["thumb"] = hand_landmarks.landmark[4].x < hand_landmarks.landmark[3].x  # thumb extended if tip left of joint (right hand)
    names = ["index", "middle", "ring", "pinky"]

    for i, tip in enumerate(finger_tips[1:]):  # loop over other 4 fingers
        # finger extended if tip higher (y lower) than joint
        fingers[names[i]] = hand_landmarks.landmark[tip].y < hand_landmarks.landmark[finger_dips[i + 1]].y

    return fingers  # dictionary of each finger True=extended, False=folded

def count_extended(finger_states):
    return sum(finger_states.values())  # count number of fingers extended

def get_stable_gesture(new_value):
    gesture_buffer.append(new_value)  # add latest finger count to buffer
    if len(gesture_buffer) < STABLE_FRAMES_REQUIRED:
        return None  # not enough frames to determine stability
    if all(v == gesture_buffer[0] for v in gesture_buffer):
        return gesture_buffer[0]  # return gesture if all recent frames match
    return None  # gesture not stable yet

# ---------------- Main Loop ----------------
while True:
    success, img = cap.read()  # read frame from webcam
    if not success:
        break  # stop loop if frame not captured

    img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)  # convert BGR to RGB for Mediapipe
    results = hands.process(img_rgb)  # detect hands and landmarks
    stable_gesture = None  # reset stable gesture for this frame

    if results.multi_hand_landmarks:  # check if any hand is detected
        handLms = results.multi_hand_landmarks[0]  # take first detected hand
        mp_draw.draw_landmarks(img, handLms, mp_hands.HAND_CONNECTIONS)  # overlay landmarks on image

        finger_states = get_finger_states(handLms)  # check which fingers are extended
        finger_count = count_extended(finger_states)  # count extended fingers
        stable_gesture = get_stable_gesture(finger_count)  # check if gesture stable across frames
    else:
        gesture_buffer.clear()  # clear buffer if no hand detected
        stable_gesture = None
        current_stable_gesture = None  # reset last stable gesture
        gesture_hold_start = None  # reset hold timer

    current_time = time.time()  # current timestamp for timing events

    # ---------------- Handle Gesture Changes ----------------
    if stable_gesture is not None:
        if stable_gesture != current_stable_gesture:  # new gesture detected
            current_stable_gesture = stable_gesture  # update current stable gesture
            gesture_hold_start = current_time  # reset hold timer
            trigger_fired = False  # reset trigger flag

        held_time = current_time - gesture_hold_start if gesture_hold_start else 0  # how long gesture has been held

        # ---------------- State Machine ----------------
        if stable_gesture == 0 and held_time >= TRIGGER_HOLD_TIME:
            # fist held long enough triggers selection mode
            if state != STATE_SELECTION:
                state = STATE_SELECTION
                selected_motor = None
                print("\n>>> ENTERED SELECTION MODE <<<\n")
        elif state == STATE_SELECTION:
            if 1 <= stable_gesture <= 5 and held_time >= TRIGGER_HOLD_TIME:
                # finger number determines motor selection after hold
                selected_motor = stable_gesture
                state = STATE_DIRECTION
                selection_time = current_time  # mark when selection made
                print(f"\n>>> MOTOR {selected_motor} SELECTED <<<\n")

    # ---------------- Continuous Console Updates ----------------
    if state == STATE_SELECTION:
        if current_stable_gesture and 1 <= current_stable_gesture <= 5:
            print(f"Holding Motor {current_stable_gesture}: {held_time:.1f}s", end="\r")  # live feedback for selection

    if state == STATE_DIRECTION and selected_motor is not None:
        if selection_time and current_time - selection_time < SELECTION_DELAY:
            # wait SELECTION_DELAY before motor starts moving
            print(f"Motor {selected_motor} → WAITING ({SELECTION_DELAY - (current_time - selection_time):.1f}s)", end="\r")
            gesture_command = 9  # idle command during wait
        else:
            # assign movement command based on gesture
            if stable_gesture == 1:
                gesture_command = 1  # forward
            elif stable_gesture == 2:
                gesture_command = 2  # backward
            elif stable_gesture == 0:
                gesture_command = 0  # stop
            else:
                gesture_command = 9  # idle if unknown gesture

            ser.write(f"{selected_motor},{gesture_command}\n".encode())  # send motor command via serial

            # console feedback for motor actions, limited by CONSOLE_UPDATE_INTERVAL
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

    cv2.imshow("Industrial Hand Tracking", img)  # display webcam frame with overlays
    if cv2.waitKey(1) & 0xFF == 27:  # break loop if ESC pressed
        break

cap.release()  # release webcam
cv2.destroyAllWindows()  # close OpenCV windows
ser.close()  # close serial port
print("Program closed cleanly.")