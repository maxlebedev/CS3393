#include <stdio.h>
#include <stdlib.h>

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

int main(int argc, char* argv[]){

	//TODO: maybe put this in a separate function
	//interpret cmd line args
	char* username;
	long port = 0;//Default value
	char client;

	parse_args(argv, &username, &port, &client);
	printf("username: %s  port: %ld  client %d\n", username, port, client);
}

