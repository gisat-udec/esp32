import tkinter as tk
import managetkeventdata as tke
import numpy as np
from matplotlib.figure import Figure
from matplotlib.backend_bases import key_press_handler
from matplotlib.backends.backend_tkagg import (
    FigureCanvasTkAgg, NavigationToolbar2Tk)
from vispy import app, scene
from vispy.color import Color


class UI_Sensor:
    def __init__(self, ui):
        self.ui = ui
        self.window = tk.Toplevel(ui, width=1000, height=500)
        self.window.withdraw()
        self.window.title("Monitor - Sensores")
        self.window.protocol("WM_DELETE_WINDOW", self.window.withdraw)

        self.canvas_imu = scene.SceneCanvas(keys='interactive', size=(
            500, 500), show=True, app="tkinter", parent=self.window)
        self.canvas_imu.native.place(width=500, height=500)
        self.view = self.canvas_imu.central_widget.add_view()
        self.view.bgcolor = Color("#003333")

        self.fig = Figure(figsize=(5, 4), dpi=100)
        self.t = np.arange(0, 3, .01)
        self.fig.add_subplot(111).plot(self.t, 2 * np.sin(2 * np.pi * self.t))
        self.canvas_mpl = FigureCanvasTkAgg(self.fig, master=self.window)
        self.canvas_mpl.draw()
        self.canvas_mpl.get_tk_widget().place(x=500, width=500, height=500)
