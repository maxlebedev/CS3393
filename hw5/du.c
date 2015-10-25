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

#define DIRSIZE 4
#define DEF_ARR_LEN 4

struct arraySet {// using a set of sorts to tell if there are hardlinks to skip
	uintmax_t* array;
	int size;
	int maxSize;
};
typedef struct arraySet arrSet;

int traverseDir(char* dirname, int count, arrSet* inodeSet);
void printDir(char* dirname, arrSet* inodeSet);
int sizeOfFile(char* dir, arrSet* inodeSet);
int contains(uintmax_t inode, arrSet* inodeSet);
void resize(arrSet* inodeSet);

int main (int argc, char* argv[]){
	arrSet inodeSet;
	inodeSet.size = 0;
	inodeSet.maxSize = DEF_ARR_LEN;
	inodeSet.array = malloc(sizeof(uintmax_t)*inodeSet.maxSize);
	
	arrSet* setptr = &inodeSet;

	char path[PATH_MAX+1];
	if(!realpath(argv[1], path)){
		perror("failed to resolve path");
	}
	printDir(path, setptr);
	return 0;
}

void printDir(char* dirname, arrSet* inodeSet){
	int count = traverseDir(dirname, 0, inodeSet);
	printf("%d %s\n", count, dirname);
}

//int buildPath(char* dest, char* path, char* entry){ }

int traverseDir(char* dirname, int count, arrSet* inodeSet){
	DIR *dirp = opendir(dirname);
	struct dirent *direntp;

	int catlen = 0; 
	char *tmpcat = NULL;
	long int cumSize = 0;
	long int size = 0;
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
				size = traverseDir(tmpcat, count +1, inodeSet);
				cumSize += size;
				printf("%ld %s\n",size, tmpcat);
				strncat(tmpcat, "/", 1);//TODO is this needed?
			}
			else{
				//check if hard links exist
				cumSize += sizeOfFile(tmpcat, inodeSet);
			}
		}
		else if (direntp->d_name[1] == '\0'){//if we are at .
			cumSize += DIRSIZE;//TODO du seems to do this
		}
	}
	free(tmpcat);
	closedir(dirp);
	return cumSize;
}

int contains(uintmax_t inode, arrSet* inodeSet){
	for(int i = 0; i < inodeSet->size; i++){
		printf("testing against %ju of %d\n", inodeSet->array[i], inodeSet->size);
		if(inode == inodeSet->array[i]){
			return 1;
		}
	}
	return 0;

}
//Takes a directory string an returns its size
int sizeOfFile(char* file, arrSet* inodeSet){
	struct stat statBuf;
	lstat(file, &statBuf);
	if(statBuf.st_nlink > 1){
		if(contains((uintmax_t) statBuf.st_ino, inodeSet)){//if we already counted it
			return 0;
		}
		else{//new, possibly repeating hardlink, add it to the 'set'
			if(inodeSet->size == inodeSet->maxSize){
				resize(inodeSet);
			}
			inodeSet->array[inodeSet->size] = statBuf.st_ino;
			inodeSet->size++;
		}
	}
	return statBuf.st_blocks;
//This is not so simple. The linux machine I tested this on had a block size of 4096
//st_blocks (and /usr/bin/stat) consistently told me files were 2x larger than what du claimed
}

//takes an arraySet, and doubles it's maximum size
void resize(arrSet* inodeSet){
	inodeSet->maxSize = inodeSet->maxSize*2;
	uintmax_t* newArr;
	newArr = malloc(sizeof(uintmax_t)*inodeSet->maxSize);

	free(inodeSet->array);
	inodeSet->array = newArr;
}







