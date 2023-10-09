import asyncio
from ui import UI
from ethernet import Ethernet


async def main():
    ui = UI()
    loop.create_task(ui.loop())
    ethernet = Ethernet(ui)
    await ethernet.init()

loop = asyncio.get_event_loop()
loop.create_task(main())
loop.run_forever()
