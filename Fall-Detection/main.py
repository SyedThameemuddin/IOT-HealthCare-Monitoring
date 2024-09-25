import cv2
import cvzone
import math
from ultralytics import YOLO

# Load YOLO model
model = YOLO('C:\\Users\\Syed Thameemuddin\\Downloads\\Fall-Detection-main\\Fall-Detection-main\\yolov8s.pt')

# Load class names
classnames = []
with open('C:\\Users\\Syed Thameemuddin\\Downloads\\Fall-Detection-main\\Fall-Detection-main\\classes.txt', 'r') as f:
    classnames = f.read().splitlines()

# Capture video from webcam (0 for default camera)
cap = cv2.VideoCapture(0)

while True:
    ret, frame = cap.read()
    if not ret:
        break  # Exit if the webcam feed cannot be read

    frame = cv2.resize(frame, (980, 740))

    results = model(frame)

    for info in results:
        parameters = info.boxes
        for box in parameters:
            x1, y1, x2, y2 = box.xyxy[0]
            x1, y1, x2, y2 = int(x1), int(y1), int(x2), int(y2)
            confidence = box.conf[0]
            class_detect = box.cls[0]
            class_detect = int(class_detect)
            class_detect = classnames[class_detect]
            conf = math.ceil(confidence * 100)

            # Implement fall detection using the coordinates x1, y1, x2
            height = y2 - y1
            width = x2 - x1
            threshold = height - width

            if conf > 80 and class_detect == 'person':
                cvzone.cornerRect(frame, [x1, y1, width, height], l=30, rt=6)
                cvzone.putTextRect(frame, f'{class_detect}', [x1 + 8, y1 - 12], thickness=2, scale=2)

            if threshold < 0:
                cvzone.putTextRect(frame, 'Fall Detected', [x1, y1 - 12], thickness=2, scale=2)

    cv2.imshow('frame', frame)
    if cv2.waitKey(1) & 0xFF == ord('t'):
        break

cap.release()
cv2.destroyAllWindows()
