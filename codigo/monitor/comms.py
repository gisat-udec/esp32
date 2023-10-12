import threading
import asyncio
from packet import Packet

PORT = 27015


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
            local_addr=('0.0.0.0', PORT))

    def tx_callback(self, data, addr):
        Packet.onpacket(self.app.ui, data)
