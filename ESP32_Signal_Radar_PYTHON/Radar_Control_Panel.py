import subprocess
import webbrowser
import tkinter as tk
from tkinter import scrolledtext
import os

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
BAT_FILE = os.path.join(BASE_DIR, "Start_Radar.bat")


# -------------------------
# Logging
# -------------------------
def log(msg):
    console.insert(tk.END, msg + "\n")
    console.see(tk.END)


# -------------------------
# Start FULL SYSTEM (BAT)
# -------------------------
def start_server():
    log("Launching radar system (BAT)...")

    try:
        subprocess.Popen(
            ["cmd.exe", "/c", BAT_FILE],
            cwd=BASE_DIR
        )

        log("Radar system launched.")
        log("Waiting for server to be ready...")

        # ✅ Auto-open browser AFTER delay
        root.after(4000, open_dashboard)

    except Exception as e:
        log(f"ERROR: {e}")


# -------------------------
# Stop Server ONLY (safe)
# -------------------------
def stop_server():
    log("Stopping radar server...")

    try:
        subprocess.call(
            'taskkill /F /FI "WINDOWTITLE eq Radar Server*"',
            shell=True
        )
        log("Server stopped. COM port should be free.")
    except Exception as e:
        log(f"ERROR stopping server: {e}")


# -------------------------
# Restart SYSTEM
# -------------------------
def restart_server():
    log("Restarting full system...")

    stop_server()

    # give COM port time to release
    root.after(2000, start_server)

    log("Restart command sent.")


# -------------------------
# Open browser
# -------------------------
def open_dashboard():
    log("Opening dashboard...")
    webbrowser.open("http://localhost:5000")


# -------------------------
# GUI
# -------------------------
root = tk.Tk()
root.title("ESP32 Radar Control Panel")
root.geometry("520x420")
root.configure(bg="#0b0f1a")

title = tk.Label(root, text="ESP32 RADAR CONTROL PANEL",
                 fg="#00eaff", bg="#0b0f1a",
                 font=("Consolas", 14, "bold"))
title.pack(pady=10)

btn_frame = tk.Frame(root, bg="#0b0f1a")
btn_frame.pack(pady=10)

btn_style = {
    "width": 18,
    "font": ("Consolas", 10, "bold"),
    "bg": "#111a2e",
    "fg": "#00eaff",
    "activebackground": "#00eaff",
    "activeforeground": "#000000"
}

tk.Button(btn_frame, text="Start System", command=start_server, **btn_style).grid(row=0, column=0, padx=5, pady=5)
tk.Button(btn_frame, text="Stop Server", command=stop_server, **btn_style).grid(row=0, column=1, padx=5, pady=5)
tk.Button(btn_frame, text="Restart System", command=restart_server, **btn_style).grid(row=1, column=0, padx=5, pady=5)
tk.Button(btn_frame, text="Open Dashboard", command=open_dashboard, **btn_style).grid(row=1, column=1, padx=5, pady=5)

console = scrolledtext.ScrolledText(root, width=60, height=15,
                                    bg="#05070d", fg="#00ff99",
                                    font=("Consolas", 9))
console.pack(pady=10)

log("System ready.")
log("GUI + Browser integration active.")

root.mainloop()
