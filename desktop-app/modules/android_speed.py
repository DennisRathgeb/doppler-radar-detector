import socket
import threading
import time


class AndroidSpeedReceiver:
    def __init__(self, port=5005, max_age=5.0):
        """
        Listens for GPS speed via UDP (from Android GPS streamer).
        :param port: Port to listen on.
        :param max_age: Max valid age of the speed in seconds.
        """
        self.port = port
        self.max_age = max_age
        self.latest_speed_kmh = None
        self.last_update_time = 0
        self._stop_flag = False

        self.thread = threading.Thread(target=self._listen_loop, daemon=True)
        self.thread.start()

    def _listen_loop(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind(("0.0.0.0", self.port))
        print(f"Listening for UDP on port {self.port}...")

        while not self._stop_flag:
            try:
                data, addr = sock.recvfrom(1024)
                text = data.decode(errors='ignore').strip()
                if text.startswith(("$GPVTG", "$GNVTG")):
                    parts = text.split(",")
                    if len(parts) > 7 and parts[7]:
                        try:
                            speed_kmh = float(parts[7])
                            self.latest_speed_kmh = speed_kmh
                            self.last_update_time = time.time()
                        except ValueError:
                            continue
            except Exception as e:
                print(f"Error in UDP listener: {e}")

    def get_speed(self):
        """Returns the latest speed if it's recent, else None."""
        if time.time() - self.last_update_time < self.max_age:
            return self.latest_speed_kmh
        return None

    def stop(self):
        self._stop_flag = True