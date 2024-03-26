import serial
import argparse
import asyncio
from bleak import BleakClient
from experiment import Experiment

CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"
ESP32_MAC_ADDRESS = "E0:E2:E6:70:42:A6"


async def run_ble(args):
    async with BleakClient(ESP32_MAC_ADDRESS) as client:
        print("Starting session.")
        exp = Experiment(args)
        while not len(exp.times) or exp.times[-1] < args.timeout:
            data = await client.read_gatt_char(CHARACTERISTIC_UUID)
            line = data.decode()
            exp.step(line)
        exp.save()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Program linking sensor system and motor driver for the purpose of controlling a soft elbow orthotic device"
    )
    parser.add_argument(
        "-o", "--output", default="test", help="Output file name for data and image"
    )
    parser.add_argument(
        "-t", "--timeout", type=int, default=5, help="Total test time in seconds"
    )
    parser.add_argument(
        "-m", "--model", default="open_model_0", help="Neural network model name"
    )
    parser.add_argument("-b", "--ble", action="store_true", help="BLE mode")
    parser.add_argument(
        "-d",
        "--debug",
        action="store_true",
        help="Debug mode, run without motor driver connection",
    )
    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose mode")

    args = parser.parse_args()

    # Configure serial port
    if args.ble:
        loop = asyncio.get_event_loop()
        loop.run_until_complete(run_ble(args))
    else:
        ser = serial.Serial("/dev/ttyUSB0", 115200)
        while not ser.in_waiting:
            continue
        print("Starting session.")
        exp = Experiment(args)
        while not len(exp.times) or exp.times[-1] < args.timeout:
            while not ser.in_waiting:
                continue
            line = ser.readline()
            exp.step(line)
        exp.save()
