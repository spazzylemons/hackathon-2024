import serial
import time

names = [
    'L1', 'C1', 'R1',
    'L2', 'C2', 'R2',
    'L3', 'C3', 'R3',
]

def run_code(ser, name):
    ser.write(b'M23 ' + name.encode() + b'.gco\r\nM24\r\n')
    ser.flush()

with serial.Serial('/dev/ttyACM0', 115200) as ser:
    time.sleep(10)

    run_code(ser, 'board')

    while True:
        try:
            line = input("Enter which spot: ")
        except EOFError:
            exit()

        try:
            line = int(line) - 1
            if 0 <= line <= 8:
                run_code(ser, 'O' + names[line])
        except:
            pass
