#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

//TODO handle 0 args (.)  or a dir
//TODO handle symlinks
//TODO handle hard links
//TODO test on linux

int traverseDir(char* dirname, int count);

int main (int argc, char* argv[]){
	traverseDir(argv[1], 0);
	return 0;
}

//TODO remember what is just a name and what is a full path
int traverseDir(char* dirname, int count){
	DIR *dirp = opendir(dirname);
	struct dirent *direntp;

	int catlen = 0; 
	char *tmpcat = NULL;
	tmpcat = (char*) malloc(sizeof(char*)*catlen);
	while((direntp = readdir(dirp)) != NULL){
		if (direntp->d_name[0] != '.') { // By default, skip files starting with '.'
			if(direntp->d_type == DT_DIR){
				//build the dir string
				catlen = strlen(dirname) + strlen(direntp->d_name) + 2;//1 extra for '/'m 1 for '\0'
				tmpcat = (char*) calloc(catlen,  sizeof(char*));
				strncat(tmpcat, dirname, strlen(dirname));
				strncat(tmpcat, direntp->d_name, strlen(direntp->d_name));


				struct stat statBuf;
				lstat(direntp->d_name, &statBuf);
				//printf("st_blksize: %lld\n",statBuf.st_blocks);
				//printf("count: %d catlen: %d tmpcat: %s\n", count, catlen, tmpcat);
				printf("%lld : %s\n", statBuf.st_blocks, tmpcat);
				strncat(tmpcat, "/", 1);
				count = traverseDir(tmpcat, count +1);
			}
			else{
				;
				//printf("%s\n", direntp->d_name);
			}
		}
	}
	free(tmpcat);
	closedir(dirp);
	return count;
}
