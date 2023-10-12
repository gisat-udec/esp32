from tkinter import *
from tkinter.ttk import *
import managetkeventdata as tke
from tkintermapview import TkinterMapView


class UI_GPS(Toplevel):
    def __init__(self, *args):
        super().__init__(*args)
        self.withdraw()
        self.protocol("WM_DELETE_WINDOW", self.withdraw)
        self.geometry("800x600")
        self.title("Monitor - GPS")

        self.map_widget = TkinterMapView(
            self, width=800, height=600, corner_radius=0)
        self.map_widget.pack(fill=BOTH, expand=YES)
