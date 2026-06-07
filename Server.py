import serial
import threading
import time
from flask import Flask, send_from_directory
from flask_socketio import SocketIO

SERIAL_PORT = "COM3"
BAUD = 115200

app = Flask(__name__, static_folder=".")
socketio = SocketIO(app, cors_allowed_origins="*")

# -----------------------------
# SERIAL CONNECTION
# -----------------------------
def connect_serial():
    while True:
        try:
            ser = serial.Serial(SERIAL_PORT, BAUD, timeout=1)
            print("Serial connected")
            return ser
        except Exception as e:
            print("Serial retrying:", e)
            time.sleep(2)

ser = connect_serial()

# -----------------------------
# PARSER
# -----------------------------
def parse(line):
    try:
        p = line.strip().split(",")
        if not p:
            return None

        msg_type = p[0]

        # ---------------- WIFI / BLE / BT ----------------
        if msg_type in ["WIFI", "BLE", "BT"]:
            if len(p) < 3:
                return None

            try:
                rssi = int(float(p[2]))
            except:
                rssi = -70

            return {
                "type": msg_type.lower(),
                "name": p[1] if len(p) > 1 and p[1] else f"{msg_type}_UNKNOWN",
                "rssi": rssi
            }

        # ---------------- IMU ----------------
        if msg_type == "IMU":
            return {
                "type": "imu",
                "yaw": float(p[3]) if len(p) > 3 else 0
            }

        # ---------------- FUSION ----------------
        if msg_type == "FUSION":
            return {
                "type": "fusion",
                "score": int(p[1]) if len(p) > 1 else 0,
                "motion": int(p[2]) if len(p) > 2 else 0,
                "presence": int(p[3]) if len(p) > 3 else 0,
                "hall": float(p[4]) if len(p) > 4 else 0,
                "emg": float(p[5]) if len(p) > 5 else 0
            }

        # 🔥 FIX: HALL SENSOR SUPPORT
        if msg_type == "HALL":
            return {
                "type": "hall",
                "value": int(p[1]) if len(p) > 1 else 0
            }

        return None

    except Exception as e:
        print("Parse error:", e)
        return None
# -----------------------------
# SERIAL LOOP
# -----------------------------
def serial_loop():
    global ser

    while True:
        try:
            line = ser.readline().decode(errors="ignore").strip()

            if not line:
                continue

            # 🔥 RAW SERIAL STREAM (FIX FOR YOUR LOG PANEL)
            socketio.emit("serial_log", {
                "line": line,
                "time": time.time(),
                "level": "normal"
            })

            data = parse(line)

            if data:
                socketio.emit("esp32_data", data)

        except Exception as e:
            print("Serial error:", e)
            ser = connect_serial()


# -----------------------------
# ROUTES
# -----------------------------
@app.route("/")
def index():
    return send_from_directory(".", "Network.html")


# -----------------------------
# START
# -----------------------------
threading.Thread(target=serial_loop, daemon=True).start()

if __name__ == "__main__":
    print("RADAR CORE ONLINE → http://localhost:5000")
    socketio.run(app, host="0.0.0.0", port=5000)
