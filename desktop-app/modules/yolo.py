import cv2
from ultralytics import YOLO

class YoloDetector:
    def __init__(self, model_path="yolov8n.pt", fov_deg=60, frame_width=640, object_classes = ["car", "person", "bicycle"]):
        """
        Initializes the YOLOv8 detector.
        :param model_path: Path to YOLOv8 model (e.g., yolov8n.pt)
        :param fov_deg: Horizontal field of view of the camera
        :param frame_width: Width of the input frames (used for angle calc)
        """
        self.model = YOLO(model_path)
        self.fov_deg = fov_deg
        self.frame_width = frame_width
        self.object_classes = object_classes

    def process_frame(self, frame):
        """
        Run YOLO on the given frame and return a list of detections.
        :param frame: OpenCV BGR frame
        :return: List of dictionaries with class, confidence, bbox, center, angle
        """
        results = self.model.predict(source=frame, conf=0.3, verbose=False)[0]
        detections = []

        for box in results.boxes:
            cls_id = int(box.cls[0])
            class_name = self.model.names[cls_id]

            if class_name not in self.object_classes:
                continue  # Skip unwanted classes

            conf = float(box.conf[0])
            x1, y1, x2, y2 = map(int, box.xyxy[0])
            cx = int((x1 + x2) / 2)
            cy = int((y1 + y2) / 2)

            # Calculate horizontal angle
            angle = ((cx - self.frame_width / 2) / self.frame_width) * self.fov_deg

            detections.append({
                "class": class_name,
                "confidence": conf,
                "bbox": (x1, y1, x2, y2),
                "center": (cx, cy),
                "angle_deg": angle
            })

        return detections
