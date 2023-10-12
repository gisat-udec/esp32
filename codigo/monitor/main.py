import asyncio
import tkinter as tk
import managetkeventdata as tke
from ui.ui import UI
from comms import Ethernet


class App:
    def __init__(self):
        self.ui = UI(self)
        self.ethernet = Ethernet(self)


app = App()
tk.mainloop()
app.ethernet.loop.call_soon_threadsafe(app.ethernet.loop.stop)
