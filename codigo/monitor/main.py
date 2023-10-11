import asyncio
import threading
import tkinter as tk
import managetkeventdata as tke
from ui import UI
from comms import Ethernet


class App:
    def __init__(self):
        self.ui = UI(self)
        self.ethernet = Ethernet(self)
        threading.Thread(target=self.thread, daemon=True).start()

    loop = asyncio.new_event_loop()

    def thread(self):
        self.loop.create_task(self.ethernet.run())
        self.loop.run_forever()


app = App()
tk.mainloop()
app.loop.call_soon_threadsafe(app.loop.stop)
