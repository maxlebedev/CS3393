/*	msh - Max's Shell
 *	Created by: Max Lebedev
 *	This is a simple shell designed to read input,
 *	tokenize it, and exec it.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LEN 100
#define MAX_TOK 10

//TODO: make it work with 1 pipe first
//TODO: we probably don't handle trailing pipes very well?

static int getLine (char *prmpt, char *buff, size_t size);
void execute(char* argv[]);
int shellLoop();
int parse(char* buff, char * argvv[][MAX_TOK]);
int countPipes(char* buff);
int builtin(char* argv[]);

void chain2(char* argvv[][MAX_TOK]){
	int fd[2];
	if (pipe(fd) < 0 ){
		perror("pipe creation failed");
	}
	int pid = fork();
	if(pid == -1){
		perror("forking  failed");
	}
	if(pid == 0){
		close(1);
		dup(fd[1]);
		execvp(argvv[0][0], argvv[0]);//pipe gets closed here
		perror("exec failed");
	}
	else{ //close write end of pipe?
		wait(NULL);
		pid = fork();
		if(pid == 0){
			close(0);
			dup(fd[0]);
			close(fd[1]);//important to close the writing pipe
			execvp(argvv[1][0], argvv[1]);
			perror("exec failed");
		}
		else{
			close(fd[1]);
			close(fd[0]);
			wait(NULL);
		}
	}
}

int main (int argc, char* argv[]){
	int done = 0;
	char* prompt = getenv("PS1");
	if(!prompt){
		prompt = "|>";
	}
	//read a line, parse it and execute it
	//char* argvv [][];
	while(!done){
		char buff[MAX_LEN];
		getLine(prompt, buff, MAX_LEN);
		int pipes = countPipes(buff);
		char* argvv[pipes+1][MAX_TOK];

		int argvCount = parse(buff, argvv);//less of a count, more of a index of last
		printf("argvCount: %d\n", argvCount);
		if(argvCount == 1){
			chain2(argvv);
		}
		else{
			for(int i = 0; i <= argvCount; i++){
				///printArray(argvv[i]);
				//check for builtins
				if(!builtin(argvv[i])){
					execute(argvv[i]);
				}
			}
		}
	}
	return 0;
}
//Read a line of input of at most MAX_LEN
int getLine (char *prompt, char *buff, size_t size){
    printf ("%s ", prompt);//fflush (stdout);
    //check for null in case stdin was closed or something
    if (fgets (buff, size, stdin) == NULL){
	    printf ("\nNo input\n");
	    exit(1);
    }
    // If it was too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if (buff[strlen(buff)-1] != '\n'){
	int ch, extra;
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
	if(extra){
		printf ("Input too long\n");
		exit(1);
	}
	else{
	       	return 0;
	}
    }
    // Otherwise remove newline, return
    buff[strlen(buff)-1] = '\0';
    return 0;
}

//spawn a child to run the inputted command with exec
void execute(char* argv[]){
	int pid = fork();
	if(pid == -1){
		perror("fork failed");
	}
	if(pid == 0){
		execvp(argv[0],argv);
		perror("exec failed");
		exit(1);
	}
	else{
		wait(NULL);
	}
}

//handle builtin stuff like exit and cd
int builtin(char* argv[]){
	if(!strcmp(argv[0], "exit"))
		exit(0);
	if(!strcmp(argv[0], "cd"))
		chdir(argv[1]);
	//not a builtin, exec
	return 0;
}

//TODO delete this and all DB prints
void printArray(char* argv[]){
	for(int i = 0; argv[i] != '\0';i++){
		printf("Array[%d]: %s\n", i, argv[i]);
	}
}

//Parse the input buffer into an argv array. Each entry in the array is a cmd
int parse(char* buff, char * argvv[][MAX_TOK]){
	int argvCount = 0;
	char* delim = " \t";
	char* token = strtok(buff, delim);
	if(token == NULL){
		perror("strtok failed");
		exit(1);
	}
	argvv[0][0] = token;
	int tokCount = 1;

	//populate the argv array with tokens taken from input
	while(NULL != token){
		token = strtok(NULL, delim);
		if(token != NULL && !strcmp(token, "|")) {//if we see a pipe, assume a new command and reset arg counter
			argvv[argvCount][tokCount] = NULL;
			argvCount++;
			tokCount = 0;
		}
		else{
			argvv[argvCount][tokCount++] = token;
		}
	}
	return argvCount;
}
int countPipes(char* buff){
	int pipes = 0;
	for(int i = 0; buff[i] != '\0';i++){
		if(buff[i]=='|'){
			pipes++;
		}
	}
	return pipes;
}


/*
//TODO maybe the lowest descendant deals with index = 0
void daisyChainExecs(char* argvv[][MAX_TOK], int index){
	if(argvv[index+1] != NULL)//if there is more to be done
		pipe();
	int pid = fork();
	if(pid == -1){
		//error
	}
	if(pid == 0){
		daisyChainExecs(argvv, index+1){
			//problem is we want to exec after daisychaining
			//if they block nicely on iO we can maybe just exec here
	}
	else{
		wait(NULL);
	}

}
*/
