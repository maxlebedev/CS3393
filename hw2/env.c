#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define ENV_OFFSET 2
extern char **environ;


// strtol with some error handling hopefully
int strtoint(char* str){
	char* end;
	int ret = strtol(str, &end, 10);
	if(str == end){ //conversion error
		fprintf(stderr, "input could not be parsed\n");
		exit(1);
	}
	return ret;
}

//returns the number of entries modified. Takes the env array with start and end(exclusive) pointers
int modifyOldEnv(char* envs[],int start,int end){
	int modified = 0;
	for(int i = start; i < end; i++){
//for i through env
		//if(NULL != strstr(envs[i], environ  //this will be n^2
	}

	return 0;
}


int populateStringArray(int entrees, char* source[], char* destination[]){
	for(int i = 0; i < entrees; i++){//TODO length error checking
		destination[i] = source[i];
	}
	return 0;
}

//print the environment and exit
void printEnv(){
	for(int i = 0; environ[i] != NULL; i++){
		puts(environ[i]);
		exit(0);
	}
}

//TODO go through each function call and make sure it errors are checked
int main(int argc,char* argv[]){
	int makeNew = 0;
	if(argc == 1){
		printEnv();
	}
	int newEnvSize = 0;
	int cmd = 0;//index of the command being run
	if(!strcmp(argv[1], "-i\0")) {
		makeNew = 1;
	}

	char *eqptr;
	//find out where the env entrees end and the command to run begins
	for (int i=1+makeNew; i< argc; i++) {
		eqptr=strchr(argv[i],'=');
		if(eqptr !=  NULL){
			newEnvSize++;
		}
		else{
			cmd=i;
			break;//no more envs
		}
	}
	//there are 2 options, if -i then just build an env array if not, copy the current env, and modify/add to it
	//char* newEnv = malloc(char*
	char** env;

	//TODO actualy have functions
	if(makeNew){
		env = (char **) malloc(sizeof(char*)*newEnvSize);
		int j = 0;
		for(int i = 2; i < cmd; i++){
			env[j] = argv[i];
			puts(env[j]);
			j++;
		}
		environ = env;
	}
	else{
		//lookup new envs in old table to see if it needs to be replaced	
		modifyOldEnv(argv, ENV_OFFSET, ENV_OFFSET+newEnvSize);
		//populateStringArray(0, environ, env);
		//then finally malloc a new env
	}

	//char** newArgv = &argv[cmd];
	char** newArgv = (char**) malloc(sizeof(char*)*(argc-cmd));
	int tmp = 0;
	for(int i = cmd; i < argc; i++){
		newArgv[tmp++] = argv[cmd];
	}


	puts("preparing to exec");
	puts(argv[cmd]);
	puts(argv[cmd+1]);
	//execvp("env ", {"env", "\0"});
	execvp(*newArgv, newArgv);//int execvp(const char *file, char *const argv[]);

	return 0;
}








