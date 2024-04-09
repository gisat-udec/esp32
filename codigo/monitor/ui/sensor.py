from tkinter import *
from tkinter.ttk import *
import managetkeventdata as tke
from raylibpy import *


class UI_Sensor(Toplevel):
    visible = False
    qx = 0
    qy = 0
    qz = 0
    qw = 0
    ax = 0
    ay = 0
    az = 0
    temperature = 0
    pressure = 0
    humidity = 0
    cylinder = False
    model = False

    def __init__(self, *args):
        super().__init__(*args)
        self.withdraw()
        tke.bind(self, "<<onsensor>>", self.onsensor)
        tke.bind(self, "<<onweather>>", self.onweather)

    def show(self):
        if self.visible:
            return
        init_window(1280, 720, "Monitor - Sensores")
        self.cylinder = gen_mesh_cylinder(1, 3, 16)
        self.model = load_model_from_mesh(self.cylinder)
        set_target_fps(60)
        self.visible = True

    def update(self):
        if not self.visible:
            return
        if window_should_close():
            close_window()
            self.visible = False
            return

        camera = Camera()
        camera.position = Vector3(0.0, -10.0, 3.0)
        camera.target = Vector3(0.0, 0.0, 1.5)
        camera.up = Vector3(0.0, 0.0, 1.0)
        camera.fovy = 45.0
        camera.projection = CAMERA_PERSPECTIVE

        with drawing():
            clear_background(RAYWHITE)
            draw_text("Actitud: \n\n i: " + str(round(self.qx, 4)) + "\n\n j: " + str(round(self.qy, 4)) +
                      "\n\nk: " + str(round(self.qz, 4)) + "\n\nw: " + str(round(self.qw, 4)), 10, 10, 32, BLACK)
            draw_text("Aceleración: \n\n x: " + str(round(self.ax, 4)) + "\n\n y: " + str(round(self.ay, 4)) +
                      "\n\nz: " + str(round(self.az, 4)), 10, 210, 32, BLACK)

            draw_text("Temperatura: " + str(round(self.temperature, 1)) + "\n\n Presión: " + str(round(self.pressure, 1)) +
                      "\n\nHumedad: " + str(round(self.humidity, 1)), 10, 420, 32, BLACK)
            with mode3d(camera):
                quat_offset = quaternion_from_axis_angle(
                    Vector3(0, 0, 1), -PI/2)
                quat_sensor = Quaternion(self.qx, self.qy, self.qz, self.qw)
                quat = quaternion_multiply(quat_sensor, quat_offset)
                self.model.transform = quaternion_to_matrix(quat)
                accel = Vector3(self.ay/30, self.ax/30, self.az/30)
                accel_rot = vector3_rotate_by_quaternion(accel, quat)
                draw_model(self.model, accel_rot, 1, SKYBLUE)
                draw_model_wires(self.model, accel_rot, 1, BLACK)

    def onsensor(self, event):
        self.qx = event.data[0]
        self.qy = event.data[1]
        self.qz = event.data[2]
        self.qw = event.data[3]
        self.ax = event.data[4]
        self.ay = event.data[5]
        self.az = event.data[6]

    def onweather(self, event):
        self.temperature = event.data[0]
        self.pressure = event.data[1]
        self.humidity = event.data[2]
