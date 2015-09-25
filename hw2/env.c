#include "env.h"

//TODO: Test on a linux machine

//returns 1 if the 'key' of the env entrees match
int keysMatch(char* str1, char* str2){
	//compare untill a string is over or we reach the '='
	for(int i = 0; str1[i] != '\0' && str2[i]!= '\0'; i++){
		if(str2[i] == '='){//the key matches
			return MATCH;
		}
		if(str1[i] != str2[i]){
			return !MATCH;
		}
	}
	return !MATCH;//new env entree did not contain an '='
}

//copies the elements of source into desination starting at desination[offset]
int populateStringArray(int offset, char* source[], char* destination[]){
	int elemCopied = 0;
	for(int j = 0;  source[j] != NULL; j++) {
		int copy = TRUE;
		//we search the destination for a key, if it is not there then we copy
		for(int i = 0; i<offset; i++){
			if(keysMatch(destination[i],source[j])){
				copy = FALSE;
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
    exit(0);
}

//check if the -i flag is passed, indicating that a new environ should be made
int makeNewEnv(char* argv[]){
    return (!strcmp(argv[1], "-i\0"));
}

//Return the index where env entrees end and the command to run begins. Also calculate size of new envs
int findCmdIndex(char* argv[], int argc, int makeNew, int* newEnvSize){
    char *eqptr;
    for (int i=1+makeNew; i< argc; i++) {
        eqptr=strchr(argv[i],'=');
        if(eqptr !=  NULL){
            (*newEnvSize)++;
        }
        else{
            return i;
        }
    }
    return argc;
}

//Ignore the old environment and build a fresh one
void buildFreshEnviron(char* argv[],int envEnd, char** newEnviron){
    int j = 0;
    //argv[0] is env, 1 is -1, we wanna start with 2
    for(int i = 2; i < envEnd; i++){
        newEnviron[j] = argv[i];
        j++;
    }
    newEnviron[j] = NULL;
}

// copy the envs in argv into the newEnviron
int copyArgvIntoNewEnviton(char* argv[], char** newEnviron, int cmd){
    int envLen = 0;
    for(int i = 1; i < cmd; i++){
        newEnviron[envLen] = argv[i];
        envLen++;
    }
    return envLen;
}

int main(int argc,char* argv[]){
	if(argc == 1){
		printEnv();
    }
    int makeNew = makeNewEnv(argv);
    int newEnvSize = 0;
	int cmd = argc;//index of the command being run

    cmd = findCmdIndex(argv, argc, makeNew, &newEnvSize);

	char** newEnviron = (char **) malloc(sizeof(char*)*newEnvSize);//this is too large in the case of modified envs
    if(newEnviron == NULL){
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }

    //If -i then just build an environ array from argv. If not, copy argv and then add missing keys from old to new environ
    if(makeNew){
        buildFreshEnviron(argv, cmd, newEnviron);
	}
	else{
        int envLen = copyArgvIntoNewEnviton(argv, newEnviron, cmd);
		populateStringArray(envLen, environ, newEnviron);
	}
	environ = newEnviron;

	if(argc == cmd){
		printEnv();
	}
    //The following requires root privileges to work. This took me far too long to figure out :(
    execvp(argv[cmd], &argv[cmd]);
	perror ("The following error occurred");
	return -1;
}






