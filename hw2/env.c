#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "env.h"


#define MATCH 1
extern char **environ;


//TODO: check all teh functions for errors
//TODO: Check variable names
//TODO: Make actual functions
//TODO: handle all cases

//TODO: Test on a linux machine
//TODO: if no cmd, print environ


//returns 1 if the 'key' of the env strings match
int keysMatch(char* str1, char* str2){
	//compare untill a string is over or we reach the '='
	for(int i = 0; str1[i] != '\0' && str2[i]!= '\0'; i++){
		if(str1[i] == '='){//the key matches
			return MATCH;
		}
		if(str1[i] != str2[i]){
			return !MATCH;
		}
	}
	return !MATCH; //something went wrong
}

//copies the non-null elements of source into desination starting at desination[offset]
int populateStringArray(int offset, char* source[], char* destination[]){
	int elemCopied = 0;
	for(int j = 0;  source[j] != NULL; j++) {//TODO length error checking;
		int copy = 1;
		//we search the destination for a key, if it is not there then we copy
		for(int i =
				0; i<offset; i++){
			if(keysMatch(destination[i],source[j])){
				copy = 0;
			}
		}
		if(copy){
			destination[offset + elemCopied] = source[j];
			elemCopied++;
		}
	}
	destination[offset + elemCopied] = "\0";//null terminate the thing
	return elemCopied;
}

//print the environment and exit
void printEnv(){
	for(int i = 0; environ[i] != NULL; i++){
		puts(environ[i]);
	}
}

int main(int argc,char* argv[]){
	int makeNew = 0;
	if(argc == 1){
		printEnv();
		return 0;
	}
	int newEnvSize = 0;
	int cmd = argc;//index of the command being run
	if(!strcmp(argv[1], "-i\0")) {
		makeNew = 1;
	}

	char *eqptr;
	//find out where the env entrees end and the command to run begins. Remember the command's index and break out
	for (int i=1+makeNew; i< argc; i++) {
		eqptr=strchr(argv[i],'=');
		if(eqptr !=  NULL){
			newEnvSize++;
		}
		else{
			cmd=i;
			break;
		}
	}


	//there are 2 options, if -i then just build an env array if not, copy the current env, and modify/add to it
	char** env;
	if(makeNew){
		env = (char **) malloc(sizeof(char*)*newEnvSize);
		int j = 0;
		for(int i = 1+makeNew; i < argc; i++){
			env[j] = argv[i];
			j++;
		}
		env[j] = NULL;
	}
	else{//TODO finish this thing. copy argv section into env
		//lookup new envs in old table to see if it needs to be replaced
		//int modified = modifyOldEnv(argv, ENV_OFFSET, ENV_OFFSET+newEnvSize);
		env = (char **) malloc(sizeof(char*)*(newEnvSize));//this is a little too large

		//copy stuff over
		int j = 0;//TODO fix overflow from nulls?
		for(int i = 1+makeNew; i < cmd; i++){//TODO doing until cmd is hard if there is no cmd
			env[j] = argv[i];
			j++;
		}

		int elemcopied = populateStringArray(j, environ, env);
		//TODO if elem copied =0 thats bad, but IDK what to do with that info

	}
	environ = env;

	if(argc == cmd){
		printEnv();
		return 0;
	}

	char** newArgv;
	newArgv = (char **) malloc(sizeof(char*)*(argc-cmd));
	int tmp = 0;
	for(int i = cmd; i < argc; i++){
		printf("i = %d\n", i);
		newArgv[tmp++] = argv[i];
	}
	newArgv[tmp] = NULL;
	puts(*newArgv);
	execvp(*newArgv, newArgv);//int execvp(const char *file, char *const argv[]);

	perror ("The following error occurred");
	return -1;
}
//execvp((argv[cmd]), &argv[cmd]);







