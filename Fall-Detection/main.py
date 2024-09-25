import cv2
import cvzone
import math
from ultralytics import YOLO

# Use 0 for the default webcam or specify your video file path
cap = cv2.VideoCapture(0)  # Change to video file path for testing with a video

# Load the YOLO model
model = YOLO('yolov8s.pt')

# Load class names
with open('classes.txt', 'r') as f:
    classnames = f.read().splitlines()

# Define constants for bed dimensions (you may need to adjust these based on your setup)
BED_TOP_LEFT = (300, 100)  # Example coordinates for the top left corner of the bed
BED_BOTTOM_RIGHT = (680, 500)  # Example coordinates for the bottom right corner of the bed

def detect_falls(box):
    """Check if a fall is detected based on bounding box dimensions."""
    x1, y1, x2, y2 = map(int, box.xyxy[0])
    height = y2 - y1
    width = x2 - x1
    return height < (0.5 * width)  # Fall detection threshold

def is_person_near_bed(x1, y1, x2, y2):
    """Check if the detected person is near the bed."""
    return (x1 < BED_BOTTOM_RIGHT[0] and x2 > BED_TOP_LEFT[0] and 
            y1 < BED_BOTTOM_RIGHT[1] and y2 > BED_TOP_LEFT[1])

def process_frame(frame):
    """Process a single frame for object detection and fall detection."""
    results = model(frame)
    fall_detected = False  # Flag to track fall detection
    near_bed = False  # Flag to track if the person is near the bed

    for info in results:
        parameters = info.boxes
        for box in parameters:
            confidence = box.conf[0]
            class_detect = classnames[int(box.cls[0])]
            conf = math.ceil(confidence * 100)

            if conf > 80 and class_detect == 'person':
                x1, y1, x2, y2 = map(int, box.xyxy[0])
                width, height = x2 - x1, y2 - y1

                # Draw bounding box and label
                cvzone.cornerRect(frame, [x1, y1, width, height], l=30, rt=6)
                cvzone.putTextRect(frame, f'{class_detect} {conf}%', [x1 + 8, y1 - 12], thickness=2, scale=2)

                # Fall detection
                if detect_falls(box):
                    fall_detected = True  # Set flag if fall is detected
                    cvzone.putTextRect(frame, 'Fall Detected', [x1, y1 - 30], thickness=2, scale=2)

                # Check if the person is near the bed
                if is_person_near_bed(x1, y1, x2, y2):
                    near_bed = True  # Person is near the bed
                    cvzone.putTextRect(frame, 'Near Bed', [x1, y1 - 50], thickness=2, scale=2)

    return frame, fall_detected, near_bed  # Return the frame and detection statuses

while True:
    ret, frame = cap.read()
    if not ret:
        print("Failed to grab frame")
        break

    frame = cv2.resize(frame, (980, 740))
    frame, fall_detected, near_bed = process_frame(frame)  # Process the frame for detection

    # Display alert if the person is not near the bed
    if not near_bed:
        cvzone.putTextRect(frame, 'Alert: Person Not Near Bed!', [50, 50], thickness=5, scale=2)

    cv2.imshow('frame', frame)

    if fall_detected:
        # Create a separate frame for fall detection message
        fall_frame = frame.copy()
        cvzone.putTextRect(fall_frame, 'Fall Detected!', [50, 50], thickness=5, scale=5)
        cv2.imshow('Fall Detected', fall_frame)

    if cv2.waitKey(1) & 0xFF == ord('t'):
        break

# Release resources
cap.release()
cv2.destroyAllWindows()
