from ui.ui import UI
from comms import Ethernet


class App():
    def __init__(self):
        self.ui = UI(self)
        self.ethernet = Ethernet(self)
        self.ui.mainloop()


if __name__ == "__main__":
    app = App()
