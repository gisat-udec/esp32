import tkinter as tk
from PIL import Image, ImageTk, ImageFile
from io import BytesIO
ImageFile.LOAD_TRUNCATED_IMAGES = True


class Camera:
    def __init__(self, ui):
        self.ui = ui
        self.window = tk.Toplevel(ui.root)
        self.window.geometry("640x480")
        self.window.protocol("WM_DELETE_WINDOW", self.on_closing)
        self.canvas = tk.Canvas(self.window, width=640, height=480)
        self.canvas.pack(fill=tk.BOTH, expand=tk.YES)
        self.canvas.create_line(0, 0, 640, 480)

    def on_closing(self):
        self.ui.camera_window = False
        self.window.destroy()

    def display(self, jpeg):
        self.image = ImageTk.PhotoImage(Image.open(BytesIO(jpeg)))
        self.canvas.create_image((0, 0), anchor=tk.NW, image=self.image)
