import asyncio


class Connection:
    def connection_made(self, transport):
        self.transport = transport

    def datagram_received(self, data, addr):
        message = data
        print('Received %r from %s' % (message, addr))
        print('Send %r to %s' % (message, addr))
        self.transport.sendto(data, addr)


class Ethernet:
    async def init(self):
        loop = asyncio.get_running_loop()
        transport, protocol = await loop.create_datagram_endpoint(
            lambda: Connection(),
            local_addr=('0.0.0.0', 27015))
