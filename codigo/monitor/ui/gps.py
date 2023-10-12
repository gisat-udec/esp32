import tkinter as tk
import managetkeventdata as tke
from tkintermapview import TkinterMapView


class UI_GPS:
    def __init__(self, ui):
        self.ui = ui
        self.window = tk.Toplevel(ui, width=800, height=600)
        self.window.withdraw()
        self.window.title("Monitor - GPS")
        self.window.protocol("WM_DELETE_WINDOW", self.window.withdraw)

        self.map_widget = TkinterMapView(
            self.window, width=800, height=600, corner_radius=0)
        self.map_widget.pack(fill=tk.BOTH, expand=tk.YES)
