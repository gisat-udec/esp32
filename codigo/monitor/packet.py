import managetkeventdata as tke
import types
import time
from collections import deque
from struct import unpack
from zfec.easyfec import Decoder

consts = types.SimpleNamespace()

consts.PACKET_SENSOR = 0
consts.PACKET_STATS = 1
consts.PACKET_PING = 2

consts.SENSOR_IMU = 0
consts.SENSOR_AMBIENT = 1
consts.SENSOR_CAMERA = 2
consts.SENSOR_GPS = 3

packet_header_start = 0
packet_header_len = 4
packet_header_end = packet_header_start + packet_header_len

sensor_header_start = packet_header_end
sensor_header_len = 8
sensor_header_end = sensor_header_start + sensor_header_len

camera_header_start = sensor_header_end
camera_header_len = 7
camera_header_end = camera_header_start + camera_header_len

camera_chunk_start = camera_header_end
camera_chunk_len = 1000
camera_chunk_end = camera_chunk_start + camera_chunk_len

camera_chunks = dict()


class Packet:
    def onpacket(ui, data):
        header = unpack("BBH", data[packet_header_start:packet_header_end])
        packet_type = header[0]
        packet_payload_len = header[2]
        if (packet_payload_len != len(data) - packet_header_len):
            print("Error: Se esperaban %d bytes de datos en el paquete, se obtuvieron %d bytes.",
                  packet_payload_len, len(data) - packet_header_len)
            return
        match packet_type:
            case consts.PACKET_SENSOR:
                Packet.onsensor(ui, data)
            case consts.PACKET_STATS:
                payload = unpack("lL", data[packet_header_end:])
                tke.event_generate(
                    ui, "<<onstats>>", {
                        "rssi": payload[0],
                        "bytes": payload[1]
                    })
            case consts.PACKET_PING:
                payload = unpack("L", data[packet_header_end:])

    def onsensor(ui, data):
        now = time.time()
        sensor_header = unpack(
            "BbhI", data[sensor_header_start:sensor_header_end])
        sensor_type = sensor_header[0]
        sensor_time = sensor_header[3]
        match sensor_type:
            case consts.SENSOR_IMU:
                payload = unpack("<fffffff", data[sensor_header_end:])
                tke.event_generate(
                    ui.sensor, "<<onsensor>>", payload)
            case consts.SENSOR_AMBIENT:
                payload = unpack("<fff", data[sensor_header_end:])
                tke.event_generate(
                    ui.sensor, "<<onweather>>", payload)
            case consts.SENSOR_CAMERA:
                camera_header = unpack(
                    "<IBBB", data[camera_header_start:camera_header_end])
                frame = camera_header[0]
                k = camera_header[1]
                v = camera_header[2]
                id = camera_header[3]
                chunk = data[camera_chunk_start:camera_chunk_end]
                if not camera_chunks.get(frame):
                    camera_chunks[frame] = {
                        "time": now,
                        "done": False,
                        "k": k,
                        "v": v,
                        "chunks": dict()
                    }
                if camera_chunks[frame]["done"]:
                    return
                if not camera_chunks[frame]["chunks"].get(id):
                    camera_chunks[frame]["chunks"][id] = chunk
                if len(camera_chunks[frame]["chunks"]) >= k:
                    camera_chunks[frame]["done"] = True
                    dec = Decoder(k, v)
                    blocks = list(
                        camera_chunks[frame]["chunks"].values())
                    blocknums = list(
                        camera_chunks[frame]["chunks"].keys())
                    decoded = dec.decode(blocks, blocknums, padlen=0)
                    tke.event_generate(
                        ui.camera, "<<onframe>>", decoded)
                # Eliminar frames antiguos
                for frame in camera_chunks.copy():
                    if camera_chunks[frame]["time"] < now - 1:
                        del camera_chunks[frame]
            case consts.SENSOR_GPS:
                payload = unpack("<fff", data[sensor_header_end:])
                latitude = payload[0]
                longitude = payload[1]
                altitude = payload[2]
                print("gps lat: ", latitude, " lon: ",
                      longitude, " alt: ", altitude)
