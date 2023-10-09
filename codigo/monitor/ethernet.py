import asyncio
from struct import *


class Connection:
    def __init__(self, eth):
        self.eth = eth

    def connection_made(self, transport):
        self.transport = transport

    def datagram_received(self, data, addr):
        self.eth.tx_callback(data, addr)


class Ethernet:
    def __init__(self, ui):
        self.ui = ui

    async def init(self):
        loop = asyncio.get_running_loop()
        transport, protocol = await loop.create_datagram_endpoint(
            lambda: Connection(self),
            local_addr=('0.0.0.0', 27015))

    def tx_callback(self, data, addr):
        # Leer cabezal del paquete
        packet_header_len = 4
        header = unpack("BBH", data[0:packet_header_len])
        packet_type = header[0]
        packet_subtype = header[1]
        packet_payload_len = header[2]
        if (packet_payload_len != len(data) - packet_header_len):
            print("Error: Se esperaban %d bytes de datos en el paquete, se obtuvieron %d bytes.",
                  packet_payload_len, len(data) - packet_header_len)
            return
        # Leer datos del paquete
        match packet_type:
            # Sensor
            case 0:
                match packet_subtype:
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
                        print(len(chunk))
            # Estadisticas
            case 1:
                payload = unpack("Ll", data[packet_header_len:])
                self.ui.stats_update({
                    "rssi": payload[1],
                    "bytes": payload[0]
                })
                # print(rssi, rx_bytes)
            # Ping
            case 2:
                payload = unpack("L", data[packet_header_len:])
                time = payload[0]
                # print(time)
