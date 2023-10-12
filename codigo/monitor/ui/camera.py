import tkinter as tk
import managetkeventdata as tke
from PIL import Image, ImageTk, ImageFile
from io import BytesIO
ImageFile.LOAD_TRUNCATED_IMAGES = True


class UI_Camera:
    def __init__(self, ui):
        self.ui = ui
        self.window = tk.Toplevel(ui)
        self.window.withdraw()
        self.window.title("Monitor - Camara")
        self.window.protocol("WM_DELETE_WINDOW", self.window.withdraw)
        self.window.geometry("640x480")
        self.window.resizable(width=False, height=False)

        self.canvas = tk.Canvas(self.window, width=640, height=480)
        self.canvas.pack(fill=tk.BOTH, expand=tk.YES)
        self.canvas.create_line(0, 0, 640, 480)

        tke.bind(ui, "<<onframe>>", self.onframe)

    def onframe(self, event):
        self.image = ImageTk.PhotoImage(Image.open(BytesIO(event.data)))
        self.canvas.create_image((0, 0), anchor=tk.NW, image=self.image)
