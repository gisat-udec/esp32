import tkinter as tk
import tkintermapview


class GPS:
    def __init__(self, ui):
        self.ui = ui
        self.window = tk.Toplevel(ui.root)
        self.window.geometry("800x600")
        self.window.protocol("WM_DELETE_WINDOW", self.on_closing)
        self.map = tkintermapview.TkinterMapView(
            self.window, width=800, height=600, corner_radius=0)
        self.map.place(relx=0.5, rely=0.5, anchor=tk.CENTER)

    def on_closing(self):
        self.ui.gps_window = False
        self.window.destroy()
