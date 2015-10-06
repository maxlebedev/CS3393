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

#define MAX_LEN 100
#define MAX_TOK 10

static int getLine (char *prmpt, char *buff, size_t size);
void execute(char* argv[]);
int shellLoop();
void parse(char* buff);

int main (int argc, char* argv[]){
	return shellLoop();
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
		perror("uh oh");
		exit(1);
	}
	else{
		wait(NULL);
	}
}

//parse the input string into either a builtin (exit) or an argv, to then be passed to execute
void parse(char* buff){
	if(!strcmp(buff, "exit"))
		exit(0);
	char* delim = " ";
	char* token = strtok(buff, delim);
	if(token == NULL){
		perror("strtok failed");
		exit(1);
	}
	int tokCount = 0;
	char* argv[MAX_TOK];
	argv[0] = token;
	//populate the argv array with tokens taken from input
	while(NULL != token){
		token = strtok(NULL, delim);
		argv[++tokCount] = token;
	}
	execute(argv);
}

int shellLoop(){
	int done = 0;
	char* prompt = getenv("PS1");
	if(!prompt){
		prompt = "|>";
	}
	//read a line, parse it and execute it
	while(!done){
		char buff[MAX_LEN];
		getLine(prompt, buff, MAX_LEN);
		parse(buff);
	}
	return 0;
}
