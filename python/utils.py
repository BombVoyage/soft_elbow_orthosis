import keras
import sys
import numpy as np
import struct
import subprocess
import csv
import os
import matplotlib.pyplot as plt


current_directory = os.path.dirname(os.path.abspath(__file__))
plt.rcParams["text.usetex"] = True


def open_image(path):
    imageViewerFromCommandLine = {
        "linux": "xdg-open",
        "win32": "explorer",
        "darwin": "open",
    }[sys.platform]
    subprocess.Popen([imageViewerFromCommandLine, path])


def save_image(data, col_names, image_name, to_open=False, verbose=False):
    assert col_names[0].lower() == "time", "First data column should be time"
    colors = ["b", "g", "r", "c", "m", "y"]
    fig, ax = plt.subplots(len(col_names) - 1, 1, figsize=(12, 8))
    for i in range(len(col_names) - 1):
        ax[i].plot(data[:, 0], data[:, i + 1], label=col_names[i + 1], color=colors[i])
    fig.legend()
    path = os.path.join(current_directory, f"figures/{image_name}.png")
    fig.savefig(path)
    if verbose:
        print(f"Figure saved to {path}")
    if to_open:
        open_image(path)


def send_int(sock, i):
    # Convert integer to bytes and send
    int_bytes = struct.pack(
        "!i", i
    )  # Convert integer to 32-bit signed integer byte representation
    sock.sendall(int_bytes)


def save_data(data, col_names, file_name, verbose=False):
    path = os.path.join(current_directory, f"../data/{file_name}.csv")
    with open(path, mode="w", newline="") as file:
        writer = csv.writer(file)
        writer.writerow(col_names)
        for data_point in data:
            writer.writerow(data_point)
    if verbose:
        print(f"Data saved to {path}")


def save_all(data_dict, name, to_open=False, verbose=False):
    data = np.concatenate(
        [np.array(col).reshape((len(col), 1)) for col in data_dict.values()], axis=1
    )
    col_names = list(data_dict.keys())
    save_data(data, col_names, name, verbose)
    save_image(data, col_names, name, to_open, verbose)


def get_model(model_name):
    return keras.models.load_model(
        os.path.join(current_directory, f"../models/{model_name}.keras")
    )
