import asyncio
import tkinter as tk
import time
from tkinter import messagebox
from collections import deque
from camera import Camera


class UI:
    root = tk.Tk()
    root.title("Monitor")

    camera_window = False

    width = 300
    height = 145
    screenwidth = root.winfo_screenwidth()
    screenheight = root.winfo_screenheight()
    alignstr = '%dx%d+%d+%d' % (width, height,
                                (screenwidth - width) / 2, (screenheight - height) / 2)
    root.geometry(alignstr)
    root.resizable(width=False, height=False)

    bCamara = tk.Button(root)
    bCamara["text"] = "Camara"
    bCamara.place(x=10, y=10, width=70, height=25)

    bGPS = tk.Button(root)
    bGPS["text"] = "GPS"
    bGPS.place(x=10, y=40, width=70, height=25)

    ilCalidad = tk.Label(root)
    ilCalidad["anchor"] = "w"
    ilCalidad["text"] = "Calidad de la señal:"
    ilCalidad.place(x=90, y=10, width=150, height=25)

    ilVelocidad = tk.Label(root)
    ilVelocidad["anchor"] = "w"
    ilVelocidad["text"] = "Velocidad de transmisión:"
    ilVelocidad.place(x=90, y=40, width=150, height=25)

    lCalidad = tk.Label(root)
    lCalidad["anchor"] = "w"
    lCalidad["text"] = "-127 dBm"
    lCalidad.place(x=230, y=10, width=60, height=25)

    lVelocidad = tk.Label(root)
    lVelocidad["anchor"] = "w"
    lVelocidad["text"] = "0 mbps"
    lVelocidad.place(x=230, y=40, width=60, height=25)

    bGrabar = tk.Button(root)
    bGrabar["text"] = "Grabar"
    bGrabar.place(x=10, y=110, width=70, height=25)

    lEstado = tk.Label(root)
    lEstado["anchor"] = "w"
    lEstado["text"] = "En espera"
    lEstado.place(x=90, y=110, width=70, height=25)

    # Registro de bytes recibidos en que tiempo
    rx_log = deque()

    def __init__(self):
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        self.root.bind("<<stats_update>>", self.stats_update)
        self.bCamara["command"] = self.new_camera_window

    def on_closing(self):
        # if messagebox.askokcancel("Salir", "Seguro que desea salir?"):
        self.root.destroy()
        asyncio.get_event_loop().stop()

    def stats_update(self, stats):
        self.lCalidad["text"] = "{0} dBm".format(stats["rssi"])
        self.rx_log.append((stats["bytes"], time.time()))

    def new_camera_window(self):
        self.camera_window = Camera(self)

    async def loop(self):
        while (True):
            sec_ago = time.time() - 1
            for i in range(0, len(self.rx_log)):
                if (self.rx_log[0][1] < sec_ago):
                    self.rx_log.popleft()
            rx_bytes = 0
            for i in range(0, len(self.rx_log)):
                rx_bytes += self.rx_log[i][0]
            self.lVelocidad["text"] = "{0:.2f} mbps".format(rx_bytes / 125000)
            self.root.update()
            await asyncio.sleep(1/60)
