#ifndef LISTENER_H
#define LISTENER_H

#define PORT 8080

int init_server(int* serverSocket);
int accept_client(int* serverSocket, int* clientSocket);
int get_cmd(int* serverSocket, int* slientSocket, int* value);
void* get_cmd_thread(void* arg);

#endif
