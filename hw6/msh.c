/*	msh - Max's Shell
 *	Created by: Max Lebedev
 *	This is a simple shell designed to read input,
 *	tokenize it, and exec it. It now also handles pipes
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <glob.h>
#include <signal.h>

#define MAX_LEN 100
#define MAX_TOK 10

//TODO signal handling
//TODO redirection
//TODO error checking (glob, signal, redirection)

static int getLine (char *prmpt, char *buff, size_t size);
void execute(char* argv[]);
int builtin(char* argv[]);
int parse(char* buff, char * argvv[][MAX_TOK]);
int countPipes(char* buff);
void dup2Wrap(int newfd, int oldfd);
void pipeExecute(int in, int out, char* argv[MAX_TOK]);
void loopExecWrapper(char* argvv[][MAX_TOK], int index);
void loopExec(int n, char* argvv[][MAX_TOK]);
void globExec(char* cmd, char* args[]);
void setupSignals();

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
		getLine(prompt, buff, MAX_LEN);
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

void int_handler (int signum) {
	puts("int'd");
}

void setupSignals(){
	//struct sigaction act;
	struct sigaction new_action, old_action;

	new_action.sa_handler = int_handler;
	sigemptyset (&new_action.sa_mask);
	new_action.sa_flags = 0;

	sigaction (SIGINT, &new_action, &old_action);
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
			chdir(getenv("HOME"));
		}
		else {
			chdir(argv[1]);
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

int globerr(const char *path, int eerrno) {
	fprintf(stderr, "%s:  %s\n",  path, strerror(eerrno));
	return 0;	/* let glob() keep going */
}

//known problems: ~ is not recognized, 
void globExec(char* cmd, char* args[]){
	int flags = 0;
	glob_t results;
	int ret;
	//call results with offset of argc. then copy argv into results
	//oor offset of 1
	results.gl_offs = 1;
	for (int i = 0; args[i] != NULL; i++) {
		flags |= (i > 1 ? GLOB_APPEND|GLOB_DOOFFS : GLOB_DOOFFS);
		ret = glob(args[i], flags, globerr, &results);
		if ( ret < 0){
			execvp(cmd, args);
		}
		printf("ret:: %d\n", ret);
		//todo arr err checking here
	}
	results.gl_pathv[0] = cmd;
	printf("pathc:: %zu\n", results.gl_pathc);
	execvp(cmd, results.gl_pathv);
}
/*
if (ret != GLOB_NOMATCH){
args[i] = results.gl_pathv[i];
}
for (int i = 0; NULL != args[i] || NULL != results.gl_pathv[i]; i++){
printf("arg:: %s\n", args[i]);
printf("globs:: %s\n", results.gl_pathv[i]);
}
}
*/

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
	dup2(in, 0);
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



