import cv2
import mediapipe as mp
import numpy as np
import subprocess
import time
import math

# --- Configuration ---
WEBCAM_ID = 0                     # Change if your webcam is not the default
MIN_DETECTION_CONFIDENCE = 0.7    # Minimum confidence value ([0.0, 1.0]) for hand detection
MIN_TRACKING_CONFIDENCE = 0.5     # Minimum confidence value ([0.0, 1.0]) for hand tracking
MAX_NUM_HANDS = 1                 # Process only one hand for simplicity

# Gesture Recognition Settings
FIST_DIST_THRESHOLD = 0.15        # Normalized distance threshold for fingertips to wrist to detect a fist (NEEDS TUNING)
GESTURE_HOLD_FRAMES = 10          # How many consecutive frames to hold the gesture
ACTION_COOLDOWN_SEC = 1.0         # Minimum seconds between executing actions

# --- MediaPipe Initialization ---
mp_hands = mp.solutions.hands
hands = mp_hands.Hands(
    static_image_mode=False,         # Process video stream
    max_num_hands=MAX_NUM_HANDS,
    min_detection_confidence=MIN_DETECTION_CONFIDENCE,
    min_tracking_confidence=MIN_TRACKING_CONFIDENCE
)
mp_drawing = mp.solutions.drawing_utils
mp_drawing_styles = mp.solutions.drawing_styles

# --- Webcam Initialization ---
cap = cv2.VideoCapture(WEBCAM_ID)
if not cap.isOpened():
    print(f"Error: Could not open webcam ID {WEBCAM_ID}")
    exit()

# --- State Variables ---
current_gesture = None
gesture_frame_count = 0
last_action_time = 0
action_triggered_this_hold = False # Prevent multiple triggers per single hold

# --- Helper Functions ---
def calculate_distance(p1, p2):
    """Calculates Euclidean distance between two landmarks (ignoring Z for simplicity here)."""
    return math.sqrt((p1.x - p2.x)**2 + (p1.y - p2.y)**2)

def run_sway_command(command):
    """Executes a sway command using swaymsg."""
    try:
        full_command = ["swaymsg"] + command.split()
        print(f"Executing: {' '.join(full_command)}")
        subprocess.run(full_command, check=True, timeout=0.5, capture_output=True) # Use capture_output to hide swaymsg output
        return True
    except FileNotFoundError:
        print("Error: 'swaymsg' command not found. Is sway running and swaymsg in your PATH?")
        # Consider exiting or disabling sway commands if this happens repeatedly
        return False
    except (subprocess.CalledProcessError, subprocess.TimeoutExpired) as e:
        print(f"Error executing swaymsg command '{command}': {e}")
        if e.stderr:
             print(f"Swaymsg stderr: {e.stderr.decode()}")
        return False

def recognize_simple_fist(landmarks):
    """
    VERY basic fist detection.
    Checks if fingertips (excluding thumb) are close to the wrist.
    RETURNS: "FIST" or "UNKNOWN"
    """
    try:
        # Fingertips: Index, Middle, Ring, Pinky
        tip_ids = [mp_hands.HandLandmark.INDEX_FINGER_TIP, mp_hands.HandLandmark.MIDDLE_FINGER_TIP,
                   mp_hands.HandLandmark.RING_FINGER_TIP, mp_hands.HandLandmark.PINKY_TIP]
        wrist = landmarks[mp_hands.HandLandmark.WRIST]

        is_fist = True
        for tip_id in tip_ids:
            tip = landmarks[tip_id]
            distance = calculate_distance(tip, wrist)
            #print(f"Tip {tip_id.name}: Dist={distance:.3f}") # Uncomment for debugging distances
            if distance > FIST_DIST_THRESHOLD:
                is_fist = False
                break # No need to check further if one finger is extended

        # Optional: Add check for thumb tip closeness to index finger base knuckle (MCP)
        # thumb_tip = landmarks[mp_hands.HandLandmark.THUMB_TIP]
        # index_mcp = landmarks[mp_hands.HandLandmark.INDEX_FINGER_MCP]
        # thumb_dist = calculate_distance(thumb_tip, index_mcp)
        # if thumb_dist > FIST_DIST_THRESHOLD * 1.2: # Allow slightly larger distance for thumb
        #     is_fist = False

        if is_fist:
            return "FIST"
        else:
            return "UNKNOWN" # Or maybe "OPEN"? Keep it simple for now.

    except IndexError:
        print("Warning: Landmark index out of bounds during gesture recognition.")
        return "UNKNOWN"


# --- Main Loop ---
print("Starting hand detection. Press 'q' to quit.")
print(f"Make a FIST and hold it for {GESTURE_HOLD_FRAMES} frames to trigger 'swaymsg workspace next'.")
print(f"Fist detection threshold (lower means fingers closer to wrist): {FIST_DIST_THRESHOLD}")

while cap.isOpened():
    success, frame = cap.read()
    if not success:
        print("Ignoring empty camera frame.")
        continue

    # Performance improvement: Make frame smaller if needed
    # frame = cv2.resize(frame, (640, 480))

    # Flip the image horizontally for a later selfie-view display
    # and convert the BGR image to RGB.
    frame = cv2.flip(frame, 1)
    rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    # To improve performance, optionally mark the image as not writeable to
    # pass by reference.
    rgb_frame.flags.writeable = False
    results = hands.process(rgb_frame)
    rgb_frame.flags.writeable = True # Not strictly needed as we draw on the BGR frame

    frame_gesture = "NONE" # Gesture detected in this specific frame

    # Draw the hand annotations on the image.
    if results.multi_hand_landmarks:
        # We set MAX_NUM_HANDS=1, so there's at most one hand
        hand_landmarks = results.multi_hand_landmarks[0]

        # Draw landmarks and connections
        mp_drawing.draw_landmarks(
            frame,
            hand_landmarks,
            mp_hands.HAND_CONNECTIONS,
            mp_drawing_styles.get_default_hand_landmarks_style(),
            mp_drawing_styles.get_default_hand_connections_style())

        # --- Gesture Recognition ---
        frame_gesture = recognize_simple_fist(hand_landmarks.landmark)

    # --- State Management & Action Triggering ---
    if frame_gesture == "FIST":
        if current_gesture == "FIST":
            gesture_frame_count += 1
        else:
            # Start counting for a new potential FIST hold
            current_gesture = "FIST"
            gesture_frame_count = 1
            action_triggered_this_hold = False # Reset trigger flag for the new hold

        # Check if gesture held long enough AND cooldown passed AND not already triggered for this specific hold
        if (gesture_frame_count >= GESTURE_HOLD_FRAMES and
                not action_triggered_this_hold):
            now = time.time()
            if (now - last_action_time) > ACTION_COOLDOWN_SEC:
                print(f"FIST detected for {gesture_frame_count} frames. Triggering action!")
                if run_sway_command("workspace next"): # Execute the command
                    last_action_time = now
                    action_triggered_this_hold = True # Mark action as done for this hold
                    # Optional: Maybe briefly reset gesture count to require releasing fist?
                    # gesture_frame_count = 0
                    # current_gesture = None
                else:
                    # Sway command failed, maybe pause for a bit or disable commands?
                    print("Sway command failed, check logs.")
                    time.sleep(2) # Pause briefly on error

            # else: # Uncomment to see cooldown message
            #     print(f"FIST held, but in cooldown ({now - last_action_time:.1f}s / {ACTION_COOLDOWN_SEC}s)")


    else: # frame_gesture is not FIST (or no hand detected)
        # Reset if the fist gesture is broken
        if current_gesture == "FIST":
             print("Fist released.")
        current_gesture = None
        gesture_frame_count = 0
        action_triggered_this_hold = False # Reset trigger flag when fist is released


    # --- Display Info ---
    display_text = f"Gesture: {current_gesture} ({gesture_frame_count})" if current_gesture else "Gesture: NONE"
    cv2.putText(frame, display_text, (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
    # Show cooldown status visually (optional)
    cooldown_remaining = max(0, ACTION_COOLDOWN_SEC - (time.time() - last_action_time))
    cv2.putText(frame, f"Cooldown: {cooldown_remaining:.1f}s", (10, 60), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 0, 0), 2)


    # --- Show Frame ---
    cv2.imshow('Sway Hand Control - FIST for Workspace Next', frame)

    # --- Exit Condition ---
    if cv2.waitKey(5) & 0xFF == ord('q'):
        print("Exiting...")
        break

# --- Cleanup ---
hands.close()
cap.release()
cv2.destroyAllWindows()
print("Script finished.")
