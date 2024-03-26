import struct
import socket


def send_int(sock, i):
    # Convert integer to bytes and send
    int_bytes = struct.pack(
        "!i", i
    )  # Convert integer to 32-bit signed integer byte representation
    sock.sendall(int_bytes)


# Create a socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect to the server
server_address = ("localhost", 8080)
client_socket.connect(server_address)


if __name__ == "__main__":
    while True:
        to_send = input("Value to send: ")
        send_int(client_socket, int(to_send))
        if not int(to_send):
            client_socket.close()
            break
