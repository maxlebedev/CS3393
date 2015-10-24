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

int traverseDir(char* dirname, int count);
int sizeOfDir(char* dir);

int main (int argc, char* argv[]){
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
	//tmpcat = (char*) malloc(sizeof(char*)*catlen);
	long int cumSize = 0;//we might overflow
	while((direntp = readdir(dirp)) != NULL){
		if (direntp->d_name[0] != '.') { // By default, skip files starting with '.'
			if(direntp->d_type == DT_DIR){ //build the dir string
				catlen = strlen(dirname) + strlen(direntp->d_name) + 2;//1 extra for '/'m 1 for '\0'
				tmpcat = (char*) calloc(catlen,  sizeof(char*));
				strncat(tmpcat, dirname, strlen(dirname));
				if(tmpcat[strlen(tmpcat)-1] != '/'){//check if end with /, -1 for 0 offset
					strncat(tmpcat, "/" , 1);
				}
				strncat(tmpcat, direntp->d_name, strlen(direntp->d_name));

				//printf("st_blksize: %lld\n",statBuf.st_blocks);
				//printf("count: %d catlen: %d tmpcat: %s\n", count, catlen, tmpcat);
				cumSize += traverseDir(tmpcat, count +1);
				printf("size: %ld : %s\n",cumSize, tmpcat);
				strncat(tmpcat, "/", 1);//TODO is this needed
			}
			else{
				struct stat statBuf;
				lstat(direntp->d_name, &statBuf);
				//printf("%jd/%llu : %s\n", (intmax_t)statBuf.st_size, (long long unsigned int) statBuf.st_blocks, direntp->d_name);//TODO review this cast
				cumSize += statBuf.st_blocks;
				//cumSize += statBuf.st_size/statBuf.st_blksize;
			}
		}
	}
	free(tmpcat);
	closedir(dirp);
	//return count;
	return cumSize;
}

//Takes a directory string an returns its size
int sizeOfDir(char* dir){
	return 0;
}
