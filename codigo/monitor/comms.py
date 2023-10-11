import time
import asyncio
import managetkeventdata as tke
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
                        x = payload[2]
                        y = payload[1]
                        z = payload[0]
                    # BME680
                    case 1:
                        payload = unpack("<fff", data[sensor_header_end:])
                        temperature = payload[2]
                        pressure = payload[1]
                        humidity = payload[0]
                    # Camara
                    case 2:
                        header_len = 8
                        chunk_len = 1000
                        camera_header = unpack(
                            "<BBBBI", data[-header_len:])
                        id = camera_header[0]
                        v = camera_header[1]
                        k = camera_header[2]
                        frame = camera_header[4]
                        chunk = data[-chunk_len - header_len:-header_len]
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
                        latitude = payload[2]
                        longitude = payload[1]
                        altitude = payload[0]
            # Estadisticas
            case 1:
                payload = unpack("Ll", data[packet_header_len:])
                tke.event_generate(
                    self.app.ui.root, "<<onstats>>", {
                        "rssi": payload[1],
                        "bytes": payload[0]
                    })
            # Ping
            case 2:
                payload = unpack("L", data[packet_header_len:])