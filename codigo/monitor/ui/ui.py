
import tkinter as tk
import managetkeventdata as tke
import time
from collections import deque

from ui.camera import UI_Camera
from ui.sensor import UI_Sensor
from ui.gps import UI_GPS


class UI:
    root = tk.Tk()
    root.geometry("400x400")
    root.title("Monitor")
    width = 300
    height = 145
    screenwidth = root.winfo_screenwidth()
    screenheight = root.winfo_screenheight()
    alignstr = '%dx%d+%d+%d' % (width, height,
                                (screenwidth - width) / 2, (screenheight - height) / 2)
    root.geometry(alignstr)
    root.resizable(width=False, height=False)

    bCamera = tk.Button(root)
    bCamera["text"] = "Camara"
    bCamera.place(x=10, y=10, width=70, height=25)

    bGPS = tk.Button(root)
    bGPS["text"] = "GPS"
    bGPS.place(x=10, y=40, width=70, height=25)

    bSensor = tk.Button(root)
    bSensor["text"] = "Sensores"
    bSensor.place(x=10, y=70, width=70, height=25)

    ilRSSI = tk.Label(root)
    ilRSSI["anchor"] = "w"
    ilRSSI["text"] = "Calidad de la señal:"
    ilRSSI.place(x=90, y=10, width=150, height=25)

    ilSpeed = tk.Label(root)
    ilSpeed["anchor"] = "w"
    ilSpeed["text"] = "Velocidad de transmisión:"
    ilSpeed.place(x=90, y=40, width=150, height=25)

    lRSSI = tk.Label(root)
    lRSSI["anchor"] = "w"
    lRSSI["text"] = "-127 dBm"
    lRSSI.place(x=230, y=10, width=60, height=25)

    lSpeed = tk.Label(root)
    lSpeed["anchor"] = "w"
    lSpeed["text"] = "0 mbps"
    lSpeed.place(x=230, y=40, width=60, height=25)

    bRecord = tk.Button(root)
    bRecord["text"] = "Grabar"
    bRecord.place(x=10, y=110, width=70, height=25)

    lRecordState = tk.Label(root)
    lRecordState["anchor"] = "w"
    lRecordState["text"] = "En espera"
    lRecordState.place(x=90, y=110, width=70, height=25)

    rx_log = deque()

    def __init__(self, app):
        self.app = app
        self.camera = UI_Camera(self)
        self.bCamera["command"] = self.camera.window.deiconify
        self.sensor = UI_Sensor(self)
        self.bSensor["command"] = self.sensor.window.deiconify
        tke.bind(self.root, "<<onstats>>", self.onstats)
        self.root.after(1, self.loop)

    def onstats(self, event):
        self.lRSSI["text"] = "{0} dBm".format(event.data["rssi"])
        self.rx_log.append((event.data["bytes"], time.time()))

    def loop(self):
        sec_ago = time.time() - 1
        for i in range(0, len(self.rx_log)):
            if (self.rx_log[0][1] < sec_ago):
                self.rx_log.popleft()
        rx_bytes = 0
        for i in range(0, len(self.rx_log)):
            rx_bytes += self.rx_log[i][0]
        self.lSpeed["text"] = "{0:.2f} mbps".format(rx_bytes / 125000)
        self.root.after(20, self.loop)
