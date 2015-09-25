#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>



#define MATCH 1
#define TRUE 1
#define FALSE 0

extern char **environ;

int keysMatch(char* str1, char* str2);
int populateStringArray(int offset, char* source[], char* destination[]);
void printEnv();
int makeNewEnv(char* argv[]);
int findCmdIndex(char* argv[], int argc, int makeNew, int* newEnvSize );
void buildFreshEnviron(char* argv[],int envEnd, char** newEnviron);
int copyArgvIntoNewEnviton(char* argv[], char** newEnviron, int cmd);