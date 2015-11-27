#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

#define NUM_THREADS 2

//interpret cmd line args
int parse_args(char* argv[], char** username, long* port, char* client){
	char * pEnd;
	for(int i = 1; argv[i] != NULL; i++){
		if(argv[i][0] != '-'){
			continue;
		}
		else{
			switch(argv[i][1]){
				case 'c': //client
					*client=1;
					break;
				case 's': //server
					*client=0;
					break;
				case 'p': //port
					*port = strtol(argv[i+1], &pEnd, 10);
					if (argv[i+1] == pEnd){
						printf("port was not a number\n");
						exit(1);
					}
					break;
				case 'u': //username
					*username = argv[i+1];
					break;
				case 'h': //help, same as default
				default: 
					printf("usage: chat -c|-s -u USERNAME [-p PORTNUM]" );
					break;
			}
		}
	}
	return 0;
}

//wrapper for socket with erro checking
int err_socket(){
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0){
		perror("error opening socket");
	}
	return sockfd;
}

//wrapper for write with error checking
void err_write(int sockfd, char* buffer){
	if (write(sockfd,buffer,strlen(buffer)) < 0){
		perror("error writing to socket");
	}
}

//wrapper for read with error checking
void err_read(int sockfd, char* buffer){
	if (read(sockfd,buffer,255) < 0){
		perror("error reading from socket");
	}
}

void *read_thread(void *client_sockfd){
	char buffer[256];
	memset(buffer, 0, sizeof(buffer));
	if (strncmp (buffer,"exit",4) ==0){
		pthread_exit(NULL);
		//TODO kill other thread too
	}
	while(strncmp (buffer,"exit",4) !=0){
		err_read((int)client_sockfd,buffer);
		//printf("%s\n",buffer);
		puts(buffer);
	}
	pthread_exit(NULL);
}

void *write_thread(void *client_sockfd){
	char buffer[256];
	memset(buffer, 0, sizeof(buffer));
	while(strncmp (buffer,"exit",4) !=0){
		fgets(buffer,255,stdin);
		err_write((int)client_sockfd,buffer);
	}
	pthread_exit(NULL);
}

//create a read thread to get messages, create a write thread to send messages
void thread_chat(int client_sockfd){
	pthread_t threads[NUM_THREADS];
	long t = 0;
	int rc;
	rc = pthread_create(&threads[t++], NULL, read_thread, (void *)client_sockfd);
	if(rc){
		perror("failed to create read thread");
	}
	rc = pthread_create(&threads[t], NULL, write_thread, (void *)client_sockfd);
	if(rc){
		perror("failed to create write thread");
	}
	pthread_exit(NULL);
}

//do client things
int setup_client(char* username, long port){
	int sockfd;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	sockfd = err_socket();
	server = gethostbyname("localhost");
	if (server == NULL) {
		fprintf(stderr,"error, no such host\n");
		exit(1);
	}
	serv_addr.sin_family = AF_INET;
	memmove((char *)&serv_addr.sin_addr.s_addr,server->h_addr_list[0], server->h_length);
	serv_addr.sin_port = htons(port);
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		perror("error connecting");
	}
	thread_chat(sockfd);
	close(sockfd);
	return 0;
}

//do server things
int setup_server(char* username, long port){
	int sockfd;
	struct sockaddr_in serv_addr, cli_addr;
	int client_sockfd;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	sockfd = err_socket();
	socklen_t clilen = sizeof(cli_addr);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))< 0){
		perror("error on binding");
		exit(1);
	}
	listen(sockfd,1);
	client_sockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if (client_sockfd < 0){
		perror("error on accept");
	}
	thread_chat(client_sockfd);
	err_write(client_sockfd,"exit");
	close(client_sockfd);
	close(sockfd);
	return 0;
}

int main(int argc, char* argv[]){
	char* username;
	long port = 3000;//Default value
	char client;

	parse_args(argv, &username, &port, &client);

	if (client){
		setup_client(username, port);
	}
	else{
		setup_server(username, port);
	}
}

