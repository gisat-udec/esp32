import tkinter as tk
import managetkeventdata as tke
from PIL import Image, ImageTk, ImageFile
from io import BytesIO
ImageFile.LOAD_TRUNCATED_IMAGES = True


class UI_Camera(tk.Toplevel):
    def __init__(self, *args):
        super().__init__(*args)
        self.withdraw()
        self.protocol("WM_DELETE_WINDOW", self.withdraw)
        self.geometry("640x480")
        self.resizable(width=False, height=False)
        self.title("Monitor - Camara")

        self.canvas = tk.Canvas(self, width=640, height=480)
        self.canvas.pack(fill=tk.BOTH, expand=tk.YES)
        self.canvas.create_line(0, 0, 640, 480)

        tke.bind(self, "<<onframe>>", self.onframe)

    def onframe(self, event):
        self.image = ImageTk.PhotoImage(Image.open(BytesIO(event.data)))
        self.canvas.create_image((0, 0), anchor=tk.NW, image=self.image)
