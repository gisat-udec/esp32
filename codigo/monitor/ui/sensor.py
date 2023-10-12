import tkinter as tk
import managetkeventdata as tke
import numpy as np
from matplotlib.figure import Figure
from matplotlib.backend_bases import key_press_handler
from matplotlib.backends.backend_tkagg import (
    FigureCanvasTkAgg, NavigationToolbar2Tk)


class UI_Sensor:
    def __init__(self, ui):
        self.ui = ui
        self.window = tk.Toplevel(ui.root)
        self.window.protocol("WM_DELETE_WINDOW", self.window.withdraw)
        self.window.geometry("640x480")
        self.window.withdraw()

        self.fig = Figure(figsize=(5, 4), dpi=100)
        self.t = np.arange(0, 3, .01)
        self.fig.add_subplot(111).plot(self.t, 2 * np.sin(2 * np.pi * self.t))

        self.canvas = FigureCanvasTkAgg(self.fig, master=self.window)
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=1)

        self.toolbar = NavigationToolbar2Tk(self.canvas, self.window)
        self.canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=1)
