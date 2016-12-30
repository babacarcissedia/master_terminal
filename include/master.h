#ifndef MASTER_H
#define MASTER_H 1

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#define BUFFER_SIZE 256
#define CLIENT_SIZE 10
#define MAX_SEND_ATTEMPT 3

typedef struct
{
	int sock; // socket file descriptor
	char* ip;	
	in_port_t port;
	int send_attempt; 
} SlaveInfo;


void* getOutput(void* psock);
void* accept_connection(void* arg);
void remove_slave(int position);
int get_command();
int send_command(char* command);
int start_server(int port);
int stop_server();
void signal_handler(int sig);
void quit();

#endif


// add command history 
// send all history command one by one to newly connected slave