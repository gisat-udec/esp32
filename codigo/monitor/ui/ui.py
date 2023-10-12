import tkinter as tk
import managetkeventdata as tke
import time
from collections import deque

from ui.camera import UI_Camera
from ui.sensor import UI_Sensor
from ui.gps import UI_GPS


class UI(tk.Tk):
    def __init__(self, app):
        super().__init__()
        self.interval_s = 1.0 / 60.0
        self.interval_ms = int(round(self.interval_s * 1000))
        self.app = app
        self.camera = UI_Camera(self)
        self.sensor = UI_Sensor(self)
        self.gps = UI_GPS(self)

        self.geometry("400x400")
        self.title("Monitor")
        width = 300
        height = 145
        screenwidth = self.winfo_screenwidth()
        screenheight = self.winfo_screenheight()
        alignstr = '%dx%d+%d+%d' % (width, height,
                                    (screenwidth - width) / 2, (screenheight - height) / 2)
        self.geometry(alignstr)
        self.resizable(width=False, height=False)

        self.bCamera = tk.Button(self)
        self.bCamera["text"] = "Camara"
        self.bCamera.place(x=10, y=10, width=70, height=25)
        self.bCamera["command"] = self.camera.window.deiconify

        self.bGPS = tk.Button(self)
        self.bGPS["text"] = "GPS"
        self.bGPS.place(x=10, y=40, width=70, height=25)
        self.bGPS["command"] = self.gps.window.deiconify

        self.bSensor = tk.Button(self)
        self.bSensor["text"] = "Sensores"
        self.bSensor.place(x=10, y=70, width=70, height=25)
        self.bSensor["command"] = self.sensor.window.deiconify

        self.ilRSSI = tk.Label(self)
        self.ilRSSI["anchor"] = "w"
        self.ilRSSI["text"] = "Calidad de la señal:"
        self.ilRSSI.place(x=90, y=10, width=150, height=25)

        self.ilSpeed = tk.Label(self)
        self.ilSpeed["anchor"] = "w"
        self.ilSpeed["text"] = "Velocidad de transmisión:"
        self.ilSpeed.place(x=90, y=40, width=150, height=25)

        self.lRSSI = tk.Label(self)
        self.lRSSI["anchor"] = "w"
        self.lRSSI["text"] = "-127 dBm"
        self.lRSSI.place(x=230, y=10, width=60, height=25)

        self.lSpeed = tk.Label(self)
        self.lSpeed["anchor"] = "w"
        self.lSpeed["text"] = "0 mbps"
        self.lSpeed.place(x=230, y=40, width=60, height=25)

        self.bRecord = tk.Button(self)
        self.bRecord["text"] = "Grabar"
        self.bRecord.place(x=10, y=110, width=70, height=25)

        self.lRecordState = tk.Label(self)
        self.lRecordState["anchor"] = "w"
        self.lRecordState["text"] = "En espera"
        self.lRecordState.place(x=90, y=110, width=70, height=25)

        self.rx_log = deque()

        tke.bind(self, "<<onstats>>", self.onstats)
        self.after(self.interval_ms, self.loop)

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
        self.after(self.interval_ms, self.loop)
