#include <csignal>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "../include/listener.h"
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

int init_server(int *serverSocket) {
  *serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (*serverSocket == -1) {
    cerr << "Error: Could not create socket\n";
    return EXIT_FAILURE;
  }

  // Bind the socket to localhost
  sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY; // Listen on all network interfaces
  serverAddr.sin_port = htons(PORT);       // Listening port
  if (bind(*serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    cerr << "Error: Bind failed\n";
    return EXIT_FAILURE;
  }

  // Listen for incoming connections
  if (listen(*serverSocket, 5) < 0) {
    cerr << "Error: Listen failed\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int accept_client(int* serverSocket, int* clientSocket){
  // Accept incoming connections
  sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);
  *clientSocket =
      accept(*serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
  if (*clientSocket < 0) {
    cerr << "Error: Accept failed\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int get_cmd(int* serverSocket, int* clientSocket, int* value){
  while (true) {
    int bytesReceived =
        recv(*clientSocket, value, sizeof(*value), 0);
    if (bytesReceived < 0) {
      cerr << "Error: Receive failed\n";
      return EXIT_FAILURE;
    } else if (bytesReceived == 0) {
      cout << "Client disconnected\n";
      return EXIT_SUCCESS;
    } else {
      // Convert from network byte order to host byte order
      *value = ntohl(*value);
      }
    }
}

void* get_cmd_thread(void* arg) {
  int serverSocket, clientSocket = 0;
  int* cmd = (int*) arg;
  long res = EXIT_SUCCESS;
    
  init_server(&serverSocket);
  accept_client(&serverSocket, &clientSocket);
  while (true) {
      int bytesReceived = recv(clientSocket, cmd, sizeof(*cmd), 0);
      if (bytesReceived < 0) {
          cerr << "Error: Receive failed\n";
          res = EXIT_FAILURE;
          break;
      } else if (bytesReceived == 0) {
          cout << "Client disconnected\n";
          break;
      } else {
          // Convert from network byte order to host byte order
          *cmd = ntohl(*cmd);
          // Prepare signal value
          union sigval sv;
          sv.sival_int = *cmd; // Set the value to be sent with the signal

          // Send real-time signal with value
          if (sigqueue(getpid(), SIGRTMIN, sv) == -1) {
              cerr << "Error: sigqueue" << endl;
              res = EXIT_FAILURE;
              break;
          }
      }
  }
  pthread_exit((void*)res);
  // Close sockets
  close(clientSocket);
  close(serverSocket);
}
