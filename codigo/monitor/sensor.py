import tkinter as tk


class Sensor:
    def __init__(self, ui):
        self.ui = ui
        self.window = tk.Toplevel(ui.root)
        self.window.geometry("200x120")
        self.window.protocol("WM_DELETE_WINDOW", self.on_closing)

        self.ilBNO080 = tk.Label(self.window)
        self.ilBNO080["anchor"] = "w"
        self.ilBNO080["text"] = "-BNO080-"
        self.ilBNO080.place(x=10, y=10, width=90, height=25)

        self.lBNO080_time = tk.Label(self.window)
        self.lBNO080_time["anchor"] = "w"
        self.lBNO080_time["text"] = "time: "
        self.lBNO080_time.place(x=10, y=30, width=90, height=25)

        self.lBNO080_x = tk.Label(self.window)
        self.lBNO080_x["anchor"] = "w"
        self.lBNO080_x["text"] = "x: "
        self.lBNO080_x.place(x=10, y=50, width=90, height=25)

        self.lBNO080_y = tk.Label(self.window)
        self.lBNO080_y["anchor"] = "w"
        self.lBNO080_y["text"] = "y: "
        self.lBNO080_y.place(x=10, y=70, width=90, height=25)

        self.lBNO080_z = tk.Label(self.window)
        self.lBNO080_z["anchor"] = "w"
        self.lBNO080_z["text"] = "z: "
        self.lBNO080_z.place(x=10, y=90, width=90, height=25)

        self.ilBME680 = tk.Label(self.window)
        self.ilBME680["anchor"] = "w"
        self.ilBME680["text"] = "-BME680-"
        self.ilBME680.place(x=100, y=10, width=90, height=25)

        self.lBME680_time = tk.Label(self.window)
        self.lBME680_time["anchor"] = "w"
        self.lBME680_time["text"] = "time: "
        self.lBME680_time.place(x=100, y=30, width=90, height=25)

        self.lBME680_temp = tk.Label(self.window)
        self.lBME680_temp["anchor"] = "w"
        self.lBME680_temp["text"] = "TEMP: "
        self.lBME680_temp.place(x=100, y=50, width=90, height=25)

        self.lBME680_pres = tk.Label(self.window)
        self.lBME680_pres["anchor"] = "w"
        self.lBME680_pres["text"] = "PRES: "
        self.lBME680_pres.place(x=100, y=70, width=90, height=25)

        self.lBME680_hmdt = tk.Label(self.window)
        self.lBME680_hmdt["anchor"] = "w"
        self.lBME680_hmdt["text"] = "HMDT: "
        self.lBME680_hmdt.place(x=100, y=90, width=90, height=25)

    def on_closing(self):
        self.ui.sensor_window = False
        self.window.destroy()

    def update_bno(self, x, y, z):
        self.lBNO080_x["text"] = "x: {0:.07f}".format(x)
        self.lBNO080_y["text"] = "y: {0:.07f}".format(y)
        self.lBNO080_z["text"] = "z: {0:.07f}".format(z)

    def update_bme(self, temp, pres, hmdt):
        self.lBME680_temp["text"] = "TEMP: {0:.02f} C".format(temp)
        self.lBME680_pres["text"] = "PRES: {0:.0f} Pa".format(pres)
        self.lBME680_hmdt["text"] = "HMDT: {0:.0f} %".format(hmdt)
