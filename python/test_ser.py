import matplotlib.pyplot as plt
import struct
import socket
import subprocess
import numpy as np
import keras
import serial
import csv
import sys
import os
from time import time

current_directory = os.path.dirname(os.path.abspath(__file__))
plt.rcParams["text.usetex"] = True


def openImage(path):
    imageViewerFromCommandLine = {
        "linux": "xdg-open",
        "win32": "explorer",
        "darwin": "open",
    }[sys.platform]
    subprocess.Popen([imageViewerFromCommandLine, path])


def send_int(sock, i):
    # Convert integer to bytes and send
    int_bytes = struct.pack(
        "!i", i
    )  # Convert integer to 32-bit signed integer byte representation
    sock.sendall(int_bytes)


def save_data(data, col_names, test_name):
    with open(
        os.path.join(current_directory, f"../data/{test_name}.csv"),
        mode="w",
        newline="",
    ) as file:
        writer = csv.writer(file)
        writer.writerow(col_names)
        for data_point in data:
            writer.writerow(data_point)


if __name__ == "__main__":
    # TODO: parse input args
    T = 35
    times = []
    angles = []
    signal = []
    avg_angles = []
    des_torque = []
    model = keras.models.load_model(
        os.path.join(current_directory, "../models/model.keras")
    )
    test_name = "test"
    if len(sys.argv) > 1:
        test_name = sys.argv[1]
    step = 0

    # Create a socket
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # Connect to the server
    server_address = ("localhost", 8080)
    client_socket.connect(server_address)
    # Configure serial port
    ser = serial.Serial("/dev/ttyUSB0", 115200)

    while not ser.in_waiting:
        continue
    print("Starting session.")
    start_time = time()

    # Main loop to continuously receive and process data
    while not len(times) or times[-1] < T:
        while not ser.in_waiting:
            continue

        # Read data from serial port
        line = ser.readline()
        times.append(time() - start_time)
        theta, semg = line.strip().split()
        angles.append(float(theta))
        signal.append(float(semg))
        step += 1

        # Check if buffer has reached 3 entries
        if step >= 3:
            avg_angles.append(np.sum(np.array(angles[-3:])) / 3)
            des_torque.append(model(np.array(signal[-3:]).reshape(1, 3)).numpy()[0])
            send_int(client_socket, step)

    # Close the socket
    client_socket.close()

    save_data(np.array(zip(angles, signal)), ["Angles", "EMG"], test_name)

    fig, ax = plt.subplots(2, 1, figsize=(12, 8))
    ax[0].plot(times[2:], avg_angles, label=r"$\theta(t)$")
    ax[1].plot(times[2:], des_torque, label="$T_{d}(t)$")
    fig.legend()
    fig.savefig(os.path.join(current_directory, f"figures/{test_name}.png"))
    openImage(os.path.join(current_directory, f"figures/{test_name}.png"))
