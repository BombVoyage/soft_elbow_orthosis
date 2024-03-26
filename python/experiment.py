import numpy as np
import socket
from time import time
from utils import save_all, get_model, send_int

MAX_SEMG = 4000
K = 1  # K gain


def inverse_model(Td, theta):
    r = 5e-2
    a = 10e-2
    b = 15e-2
    R = np.array([[np.cos(theta), -np.sin(theta)], [np.sin(theta), np.cos(theta)]])
    va = R.dot(np.array([0, a]).T)
    vb = np.array([0, b]).T
    vab = vb - va
    return r * Td * np.linalg.norm(vab) / np.linalg.norm(np.cross(va, vab))


class Experiment:
    def __init__(self, args):
        self.args = args
        self.times = []
        self.angles = []
        self.signal = []
        self.avg_angles = []
        self.des_torque = []
        self.command = [0, 0]
        self.model = get_model(args.model)
        self.iteration = 0
        self.client_socket = None
        if not args.debug:
            self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            server_address = ("localhost", 8080)
            self.client_socket.connect(server_address)
        self.start_time = time()

    def __del__(self):
        if not self.args.debug:
            self.client_socket.close()

    def step(self, line):
        self.times.append(time() - self.start_time)
        theta, semg = line.strip().split()
        self.angles.append(float(theta))
        self.signal.append(float(semg))
        self.iteration += 1

        if self.iteration >= 3:
            self.avg_angles.append(np.sum(np.array(self.angles[-3:])) / 3)
            # Torque estimation
            if self.model.input_shape == (None, 3):
                self.des_torque.append(
                    self.model(
                        np.array(self.signal[-3:]).reshape(1, 3) / MAX_SEMG
                    ).numpy()[0][0]
                )
            elif self.model.input_shape == (None, 4):
                input = np.concatenate(
                    (np.array(self.signal[-3:]) / MAX_SEMG, [self.avg_angles[-1]])
                ).reshape(1, 4)
                self.des_torque.append(self.model(input).numpy()[0][0])
            self.command.append(
                inverse_model(self.des_torque[-1], self.avg_angles[-1]) * K
            )
            if not self.args.debug:
                send_int(self.client_socket, self.command[-1])
            if self.args.verbose:
                print(
                    f"Received: {self.angles[-1], self.signal[-1]} Command: {self.command[-1]}"
                )

    def save(self):
        save_all(
            {
                "time": self.times,
                r"$\theta$": self.angles,
                "sEMG": self.signal,
                "u": self.command,
            },
            self.args.output,
            to_open=True,
            verbose=self.args.verbose,
        )
