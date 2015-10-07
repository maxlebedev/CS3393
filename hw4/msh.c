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
//TODO: we probably don't handle trailing pipes very well

static int getLine (char *prmpt, char *buff, size_t size);
void execute(char* argv[]);
int shellLoop();
void parse(char* buff);
void parse2(char* buff, char * argvv[][MAX_TOK]);
int countPipes(char* buff);

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
		//int pipes = countPipes(buff);
		//char* argvv[pipes+1][MAX_TOK];

		parse(buff);
		/*execute();
		 if(argvv != NULL)
		 	free(argvv);
			*/
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

//parse the input string into either a builtin (exit) or an argv, to then be passed to execute TODO just parse
//TODO builtin 
void parse(char* buff){
	char* delim = " \t";
	char* token = strtok(buff, delim);
	if(token == NULL){
		perror("strtok failed");
		exit(1);
	}
	int tokCount = 0;
	char* argv[MAX_TOK];
	argv[0] = token;
	//check for builtins
	
	//populate the argv array with tokens taken from input
	while(NULL != token){
		token = strtok(NULL, delim);
		argv[++tokCount] = token;
	}
	if(!builtin(argv)){
		execute(argv);
	}
}

//Parse the input buffer into an argv array. Each entry in the array is a cmd
void parse2(char* buff, char * argvv[][MAX_TOK]){
	int argvCount = 0;
	char* delim = " \t";
	char* token = strtok(buff, delim);
	if(token == NULL){
		perror("strtok failed");
		exit(1);
	}
	int tokCount = 0;
	argvv[0][0] = token;
	//check for builtins

	//populate the argv array with tokens taken from input
	while(NULL != token){
		token = strtok(NULL, delim);
		if(!strcmp(token, "|")) {
			argvCount++;
		}
		argvv[argvCount][++tokCount] = token;
	}
	for(int i = 0; i < argvCount; i++){
		if(!builtin(argvv[i])){
			execute(argvv[i]);
		}
	}
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
