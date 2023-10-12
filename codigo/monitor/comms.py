import managetkeventdata as tke
import asyncio
import threading
import time
from zfec.easyfec import Decoder
from struct import unpack


class Ethernet:
    class Connection:
        def __init__(self, eth):
            self.eth = eth

        def connection_made(self, transport):
            self.transport = transport

        def datagram_received(self, data, addr):
            self.eth.tx_callback(data, addr)

    def __init__(self, app):
        self.app = app
        threading.Thread(target=self.thread, daemon=True).start()

    def thread(self):
        self.loop = asyncio.new_event_loop()
        self.loop.create_task(self.run())
        self.loop.run_forever()

    async def run(self):
        loop = asyncio.get_running_loop()
        transport, protocol = await loop.create_datagram_endpoint(
            lambda: self.Connection(self),
            local_addr=('0.0.0.0', 27015))

    camera_chunks = dict()

    def tx_callback(self, data, addr):
        now = time.time()
        # Leer cabezal
        packet_header_start = 0
        packet_header_len = 4
        packet_header_end = packet_header_start + packet_header_len
        header = unpack("BBH", data[packet_header_start:packet_header_end])
        packet_type = header[0]
        packet_payload_len = header[2]
        if (packet_payload_len != len(data) - packet_header_len):
            print("Error: Se esperaban %d bytes de datos en el paquete, se obtuvieron %d bytes.",
                  packet_payload_len, len(data) - packet_header_len)
            return
        # Leer datos del paquete
        match packet_type:
            # Sensor
            case 0:
                sensor_header_start = packet_header_end
                sensor_header_len = 8
                sensor_header_end = sensor_header_start + sensor_header_len
                sensor_header = unpack(
                    "BbhI", data[sensor_header_start:sensor_header_end])
                sensor_type = sensor_header[0]
                sensor_time = sensor_header[3]
                match sensor_type:
                    # BNO080
                    case 0:
                        payload = unpack("<fff", data[sensor_header_end:])
                        x = payload[0]
                        y = payload[1]
                        z = payload[2]
                    # BME680
                    case 1:
                        payload = unpack("<fff", data[sensor_header_end:])
                        temperature = payload[0]
                        pressure = payload[1]
                        humidity = payload[2]
                    # Camara
                    case 2:
                        camera_header_start = sensor_header_end
                        camera_header_len = 7
                        camera_header_end = camera_header_start + camera_header_len
                        camera_chunk_start = camera_header_end
                        camera_chunk_len = 1000
                        camera_chunk_end = camera_chunk_start + camera_chunk_len
                        camera_header = unpack(
                            "<IBBB", data[camera_header_start:camera_header_end])
                        frame = camera_header[0]
                        k = camera_header[1]
                        v = camera_header[2]
                        id = camera_header[3]
                        chunk = data[camera_chunk_start:camera_chunk_end]
                        if not self.camera_chunks.get(frame):
                            self.camera_chunks[frame] = {
                                "time": now,
                                "done": False,
                                "k": k,
                                "v": v,
                                "chunks": dict()
                            }
                        if self.camera_chunks[frame]["done"]:
                            return
                        if not self.camera_chunks[frame]["chunks"].get(id):
                            self.camera_chunks[frame]["chunks"][id] = chunk
                        if len(self.camera_chunks[frame]["chunks"]) >= k:
                            self.camera_chunks[frame]["done"] = True
                            dec = Decoder(k, v)
                            blocks = list(
                                self.camera_chunks[frame]["chunks"].values())
                            blocknums = list(
                                self.camera_chunks[frame]["chunks"].keys())
                            decoded = dec.decode(blocks, blocknums, padlen=0)
                            tke.event_generate(
                                self.app.ui.root, "<<onframe>>", decoded)
                        # Eliminar frames antiguos
                        for frame in self.camera_chunks.copy():
                            if self.camera_chunks[frame]["time"] < now - 1:
                                del self.camera_chunks[frame]
                    case 3:
                        payload = unpack("<fff", data[sensor_header_end:])
                        latitude = payload[0]
                        longitude = payload[1]
                        altitude = payload[2]
            # Estadisticas
            case 1:
                payload = unpack("lL", data[packet_header_len:])
                tke.event_generate(
                    self.app.ui.root, "<<onstats>>", {
                        "rssi": payload[0],
                        "bytes": payload[1]
                    })
            # Ping
            case 2:
                payload = unpack("L", data[packet_header_len:])
