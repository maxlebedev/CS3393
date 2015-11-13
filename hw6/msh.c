/*	msh - Max's Shell
 *	Created by: Max Lebedev
 *	This is a simple shell designed to read input,
 *	tokenize it, and exec it. It now also handles pipes
 *
 */

#define _POSIX_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <glob.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>

#define MAX_LEN 100
#define MAX_TOK 10

static int getLine (char *prmpt, char *buff, size_t size);
void execute(char* argv[]);
int builtin(char* argv[]);
int parse(char* buff, char * argvv[][MAX_TOK]);
int countPipes(char* buff);
void dup2Wrap(int newfd, int oldfd);
void pipeExecute(int in, int out, char* argv[MAX_TOK]);
void loopExecWrapper(char* argvv[][MAX_TOK], int index);
void loopExec(int n, char* argvv[][MAX_TOK]);
int globerr(const char *path, int eerrno);
void globExec(char* cmd, char* args[]);
void setupSignals();
void setupRedirects(char* argv[]);
int openWrapper(char* file,int flags,mode_t mode);

int main(int argc, char* argv[]){
	setupSignals();
	int done = 0;
	char* prompt = getenv("PS1");
	if(!prompt){
		prompt = "|>";
	}
	//read a line, parse it and execute it
	while(!done){
		char buff[MAX_LEN];
		if(!getLine(prompt, buff, MAX_LEN)){
			continue;//if input was empty try again
		}
		int pipes = countPipes(buff);
		char* argvv[pipes+1][MAX_TOK];

		int pipeCount = parse(buff, argvv);//less of a count, more of a index of last
		if(pipeCount > 0){
			loopExecWrapper(argvv,pipeCount+1);
		}
		else{
			for(int i = 0; i <= pipeCount; i++){
				if(!builtin(argvv[i])){ //check for builtins
					execute(argvv[i]);
				}
			}
		}
	}
	return 0;
}

// signal handler. it handles signals
void handle_signal(int signal) {
	switch (signal) {
		case SIGINT:
			printf("\n");
			break;
		case SIGQUIT:
			break;
	}
}

//basically ignore SIGINT and SIGQUIT
void setupSignals(){
	struct sigaction new_action;
	struct sigaction old_action;

	new_action.sa_handler = &handle_signal;
	sigemptyset (&new_action.sa_mask);
	new_action.sa_flags = 0;

	//the hw only mentioned these two signals, so these are the only two dealt with
	if(sigaction(SIGINT, &new_action, &old_action)){
		perror("Error: cannot handle SIGINT");
	}
	if(sigaction(SIGQUIT, &new_action, &old_action)){
		perror("Error: cannot handle SIGQUIT");
	}
}

//Read a line of input of at most MAX_LEN. Return number of chars read
int getLine (char *prompt, char *buff, size_t size){
	printf ("%s ", prompt);//fflush (stdout);
	//check for null in case stdin was closed or something
	//If the input is Null, return 
	if (fgets (buff, size, stdin) == NULL){
		return 0;
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
	return strlen(buff);
}

//spawn a child to run the inputted command with exec
void execute(char* argv[]){
	int pid = fork();
	if(pid == -1){
		perror("fork failed");
	}
	if(pid == 0){
		setupRedirects(argv);
		globExec(argv[0],argv);
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
	if(!strcmp(argv[0], "cd")){
		if(NULL == argv[1]){
			if(chdir(getenv("HOME"))){
				perror("chdir dailed");
			}
		}
		else {
			if(chdir(argv[1])){
				perror("chdir dailed");
			}
		}
	}
	//not a builtin, exec
	return 0;
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
		//if we see a pipe, assume a new command and reset arg counter
		if(token != NULL && !strcmp(token, "|")) {
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

//return the number of pipes. 
int countPipes(char* buff){
	int pipes = 0;
	for(int i = 0; buff[i] != '\0';i++){
		if(buff[i]=='|'){
			pipes++;
		}
	}
	return pipes;
}

//call dup2 and check for errors
void dup2Wrap(int newfd, int oldfd){
	if(dup2(newfd, oldfd)<0){
		perror("dup2 failed");
	}
}

//remove n strings from the string array arr starting at location start
void removeFromArray(int start, int n,char* arr[]){
	//if the two we are removing are at the end
	if (arr[start+n+1] == NULL){
		arr[start+1] = NULL;
	}	
	else{
		int i = start;
		while(arr[i+n] != NULL){
			arr[i] = arr[i+n];
			i++;
		}
		arr[i] = NULL;

	}
}

//a wrapper for open that also does error checking
int openWrapper(char* file,int flags,mode_t mode){
	int fd;
	if(mode){
		fd = open(file, O_RDONLY, mode);
	}
	else{
		fd = open(file, O_RDONLY);
	}
	if(fd < 0){
		perror("failed to open");
	}
	return fd;
}

//loop through argv, if >, >>, or < handle appropriaty
void setupRedirects(char* argv[]){
	int fd;
	for(int i = 0; argv[i] != NULL; i++){
		if(argv[i][0] == '<'){ //handle open, 0
			fd = open(argv[i+1], O_RDONLY, 0);
			if(fd < 0){
				perror("failed to open");
			}
			dup2Wrap(fd,0);
			removeFromArray(i, 2, argv);
			i--;//reread
		}
		else if(argv[i][0] == '>'){
			if (argv[i][1] == '>'){
				fd = open(argv[i+1], O_CREAT | O_WRONLY | O_APPEND, 666);
				if(fd < 0){
					perror("failed to open");
				}
			}
			else{
				fd = open(argv[i+1], O_CREAT | O_WRONLY | O_TRUNC, 666);
				if(fd < 0){
					perror("failed to open");
				}
			}
			dup2Wrap(fd,1);
			removeFromArray(i, 2, argv);
			i--;//reread
		}
	}
}

//errorprint function to provide to glob
int globerr(const char *path, int eerrno) {
	fprintf(stderr, "%s:  %s\n",  path, strerror(eerrno));
	return 0;	/* let glob() keep going */
}

//exec wrapper globs the input before passing it to exec
void globExec(char* cmd, char* args[]){
	int flags = 0;
	glob_t results;
	int ret;
	char* globbedArgs[MAX_LEN];
	int glInd = 0;
	for (int i = 0; args[i] != NULL; i++) {
		ret = glob(args[i], flags, globerr, &results);
		if(!ret){
			for(int j = 0; j < results.gl_pathc; j++){
				globbedArgs[glInd++] = results.gl_pathv[j];
			}
		}
		if (ret == GLOB_NOMATCH){
			globbedArgs[glInd++] = args[i];
		}
	}
	globbedArgs[glInd] = NULL;
	execvp(cmd, globbedArgs);
}

//execute, but takes an in and out rather than using stdin/stdout
void pipeExecute(int in, int out, char* argv[MAX_TOK]){
	int pid = fork();
	if(pid == 0){
		if(in != 0){
			dup2Wrap(in, 0);
			close(in);
		}
		if(out != 1){
			dup2Wrap(out, 1);
			close(out);
		}
		setupRedirects(argv);
		globExec(argv[0], argv);
		perror("exec failed");
	}
}

//run the execs in a for loop
void loopExec(int n, char* argvv[][MAX_TOK]){
	int fd [2];
	int in = 0;
	//pass along the pipe reference from 'in' 
	for(int i = 0; i < n-1 && argvv[i] != NULL; i++) {
		if(!pipe(fd)){
			perror("failed to create pipe");
		}
		pipeExecute(in, fd[1], argvv[i]);
		close (fd[1]);
		in = fd[0];
	}
	dup2Wrap(in, 0);
	setupRedirects(argvv[n-1]);
	globExec(argvv[n-1][0], argvv[n-1]);
	perror("exec failed");
}

//wrapper function the the pipe exec.
//IMO it is slightly neater to not have the shell aware of the pipe fds and such
void loopExecWrapper(char* argvv[][MAX_TOK], int index){
	int pid = fork();
	if(pid == -1){
		perror("failed to fork");
	}
	if(pid == 0){
		loopExec(index, argvv);
	}
	else{
		wait(NULL);
	}
}
