import serial
import io
from serial.tools import list_ports


def main():
    # Connect to a device matching what we want
    port = list(list_ports.grep(r"/dev/cu.usbmodem*"))[0]
    ser = serial.Serial(port.device, 115200, timeout=1)
    sio = io.TextIOWrapper(io.BufferedRWPair(ser, ser))

    # Write our command and send immediately
    sio.write("hello\n")
    sio.flush()

    # Our serial implementation prints the input to the shell
    command = sio.readline()
    # Read the response on the next line
    response = sio.readline()
    # output to user
    print(response)

if __name__ == "__main__":
    main()
