import cv2
from modules.yolo import YoloDetector
from modules.android_speed import AndroidSpeedReceiver

# Create instances
detector = YoloDetector(fov_deg=60)

speedreceiver = AndroidSpeedReceiver(port=5005)

# Open webcam
cap = cv2.VideoCapture(0)
frame_width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
detector.frame_width = frame_width  # Update actual width

def add_result2frame(frame, detections, speed, y_limit_prop=0.05):
    
    bar_y1 = int(frame.shape[0] * y_limit_prop)  
    bar_y2 = int(frame.shape[0] * (1-y_limit_prop))  # 95% from top
    for det in detections:
        cx, cy = det["center"]
        x1, _, x2, _  = det["bbox"]
        angle = det["angle_deg"]
        obj_class = det["class"]
        conf = det["confidence"]

        # Draw horizontal rectangle across full width
        cv2.rectangle(frame, (x1, bar_y1), (x2, bar_y2), (0, 255, 0), 1)

        # Draw enlarged center point
        #cv2.circle(frame, (cx, cy), 8, (0, 0, 255), -1)

        # Labels
        angle_label = f"{angle:+.1f}Deg"
        speed_label = f"{speed:.1f} km/h" if speed is not None else "No speed"
        class_label = obj_class

        # Draw angle above, speed below the point
        cv2.putText(frame, angle_label, (cx - 30, cy - 20),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 255), 2)

        cv2.putText(frame, class_label, (cx - 30, cy),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

        cv2.putText(frame, speed_label, (cx - 30, cy + 25),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 0), 2)

        # Console log
        print(f"{obj_class} | {angle:+.1f}° | {speed_label} | conf: {conf:.2f}")
while True:
    #get speed
    speed = speedreceiver.get_speed()

    #capture frame
    ret, frame = cap.read()
    if not ret:
        break
    #detect objects
    detections = detector.process_frame(frame)
    
    
    add_result2frame(frame, detections, speed)

    

    cv2.imshow("Angle Detector", frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
