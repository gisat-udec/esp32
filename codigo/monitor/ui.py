import asyncio
import tkinter as tk


class UI:
    root = tk.Tk()
    root.title("Monitor")
    # setting window size
    width = 300
    height = 145
    screenwidth = root.winfo_screenwidth()
    screenheight = root.winfo_screenheight()
    alignstr = '%dx%d+%d+%d' % (width, height,
                                (screenwidth - width) / 2, (screenheight - height) / 2)
    root.geometry(alignstr)
    root.resizable(width=False, height=False)

    bCamara = tk.Button(root)
    bCamara["text"] = "Camara"
    bCamara.place(x=10, y=10, width=70, height=25)

    bGPS = tk.Button(root)
    bGPS["text"] = "GPS"
    bGPS.place(x=10, y=40, width=70, height=25)

    ilCalidad = tk.Label(root)
    ilCalidad["anchor"] = "w"
    ilCalidad["text"] = "Calidad de la señal:"
    ilCalidad.place(x=90, y=10, width=150, height=25)

    ilVelocidad = tk.Label(root)
    ilVelocidad["anchor"] = "w"
    ilVelocidad["text"] = "Velocidad de transmisión:"
    ilVelocidad.place(x=90, y=40, width=150, height=25)

    lCalidad = tk.Label(root)
    lCalidad["anchor"] = "w"
    lCalidad["text"] = "label"
    lCalidad.place(x=240, y=10, width=50, height=25)

    lVelocidad = tk.Label(root)
    lVelocidad["anchor"] = "w"
    lVelocidad["text"] = "label"
    lVelocidad.place(x=240, y=40, width=50, height=25)

    bGrabar = tk.Button(root)
    bGrabar["text"] = "Grabar"
    bGrabar.place(x=10, y=110, width=70, height=25)

    lEstado = tk.Label(root)
    lEstado["anchor"] = "w"
    lEstado["text"] = "En espera"
    lEstado.place(x=90, y=110, width=70, height=25)

    async def loop(self):
        while (True):
            self.root.update()
            await asyncio.sleep(1/60)
