#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>

#include <inttypes.h>

//TODO handle symlinks
//TODO handle hard links

#define DIRSIZE 4
#define DEF_ARR_LEN 4

int traverseDir(char* dirname, int count);
int sizeOfFile(char* dir);

int main (int argc, char* argv[]){
	int inodeArray[DEF_ARR_LEN] = {0};
	int inodeSize = 0;
	char path[PATH_MAX+1];
	if(!realpath(argv[1], path)){
		perror("failed to resolve path");
	}
	traverseDir(path, 0);
	return 0;
}

int traverseDir(char* dirname, int count){
	DIR *dirp = opendir(dirname);
	struct dirent *direntp;

	int catlen = 0; 
	char *tmpcat = NULL;
	long int cumSize = 0;//we might overflow
	while((direntp = readdir(dirp)) != NULL){
		if (direntp->d_name[0] != '.') { // Skip files starting with '.'
			catlen = strlen(dirname) + strlen(direntp->d_name) + 1;//1 extra for '/'
			tmpcat = (char*) calloc(catlen,  sizeof(char*));
			strncat(tmpcat, dirname, strlen(dirname));
			if(tmpcat[strlen(tmpcat)-1] != '/'){//check if end with /, -1 for 0 offset
				strncat(tmpcat, "/" , 1);
			}
			strncat(tmpcat, direntp->d_name, strlen(direntp->d_name));
			if(direntp->d_type == DT_DIR){ //build the dir string
				cumSize = traverseDir(tmpcat, count +1);
				printf("size: %ld : %s\n",cumSize, tmpcat);
				cumSize = 0;
				strncat(tmpcat, "/", 1);//TODO is this needed?
			}
			else{
				printf("type: %d\n", direntp->d_type);
				int add = sizeOfFile(tmpcat);
				cumSize +=  add;
			}
		}
		else if (direntp->d_name[1] == '\0'){//if we are at .
			cumSize += DIRSIZE;//TODO check if true on all platforms du seems to do this
		}
		
	}
	free(tmpcat);
	closedir(dirp);
	return cumSize;
}

//Takes a directory string an returns its size
int sizeOfFile(char* file){
	struct stat statBuf;
	lstat(file, &statBuf);
	return statBuf.st_blocks;
//This is not so simple. The linux machine I tested this on had a block size of 4096
//st_blocks (and /usr/bin/stat) consistent told me files were 2x larger than what du claimed
}
