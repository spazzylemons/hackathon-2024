from asyncio import create_task, gather, get_event_loop, Queue, sleep, QueueFull
from serial_asyncio import open_serial_connection

PRINTER = '/dev/ttyACM0'
ARDUINO = '/dev/ttyACM1'

WIN_SEQUENCES = (
    (b'95', b'145', b'155', b'145'),
    (b'95', b'125', b'155', b'125'),
    (b'95', b'105', b'155', b'105'),
    (b'105', b'95', b'105', b'155'),
    (b'125', b'95', b'125', b'155'),
    (b'145', b'95', b'145', b'155'),
    (b'95', b'155', b'155', b'95'),
    (b'95', b'95', b'155', b'155'),
)

async def sendwin(pwriter, wintype):
    x1, y1, x2, y2 = WIN_SEQUENCES[wintype]
    pwriter.write(b'G0 X' + x1 + b' Y' + y1 + b' Z0.2\r\n')
    pwriter.write(b'G0 F1500\r\n')
    pwriter.write(b'G1 X' + x2 + b' Y' + y2 + b' Z0.2 E12.5\r\n')
    await pwriter.drain()

async def doqueries(pwriter):
    while True:
        pwriter.write(b'M27\r\n')
        await pwriter.drain()
        await sleep(2.0)

async def readprints(areader, queue):
    while True:
        line = await areader.readline()
        if line.startswith(b'PRINT'):
            print('got print message')
            name = line.strip().split()[1]
            print(f'placing {name} into queue')
            await queue.put(name)
        # elif line.startswith(b'WIN'):
        #     print('got print message')
        #     name = line.strip().split()[1]
        #     await queue.put(int(name))

async def querystatus(preader, printmsg):
    while True:
        line = (await preader.readline()).lower()
        print(line)
        try:
            if line.startswith(b'not sd printing'):
                print('Nope!')
                printmsg.put_nowait(False)
            elif line.startswith(b'sd printing'):
                print('Yep!')
                printmsg.put_nowait(True)
        except QueueFull:
            pass

async def writeprints(pwriter, printmsg, queue):
    while True:
        name = await queue.get()
        while await printmsg.get():
            print('...')
            pass
        print('Sending print...')
        # if isinstance(name, int):
        #     await sendwin(pwriter, name)
        #     await sleep(10.0)
        # else:
        pwriter.write(b'M23 ' + name + b'\r\nM24\r\n')
        await pwriter.drain()

async def run():
    areader, _ = await open_serial_connection(url=ARDUINO, baudrate=9600)
    preader, pwriter = await open_serial_connection(url=PRINTER, baudrate=115200)

    # Sleep for a bit since the system needs to warm up before we can begin
    # Might be better to actually listen in for a start event but whatevs
    await sleep(10.0)

    printmsg = Queue(1)
    queue = Queue()

    tasks = []
    tasks.append(create_task(doqueries(pwriter)))
    tasks.append(create_task(readprints(areader, queue)))
    tasks.append(create_task(querystatus(preader, printmsg)))
    tasks.append(create_task(writeprints(pwriter, printmsg, queue)))
    await gather(*tasks)

loop = get_event_loop()
loop.run_until_complete(run())
loop.close()
