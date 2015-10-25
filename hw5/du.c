#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <inttypes.h>

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
void buildPath(char* dest, char* path, char* entry);

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

//traverse the directory structure, and print the sizes off the dirs
void printDir(char* dirname, arrSet* inodeSet){
	int count = traverseDir(dirname, 0, inodeSet);
	printf("%d %s\n", count, dirname);
}

//build up the dest string with the new path
void buildPath(char* dest, char* path, char* entry){
	strncat(dest, path, strlen(path));
	//if(dest[strlen(dest)-1] != '/'){//it was at somepoint necessary to add '/'
	strncat(dest, "/" , 1);// conditionally, but I don't think that is the case anymore
	strncat(dest, entry, strlen(entry));
}

//move through each subdir and count their size in blocks
int traverseDir(char* dirname, int count, arrSet* inodeSet){
	DIR *dirp = opendir(dirname);
	struct dirent *direntp;

	int catlen = 0, cumSize = 0,size = 0;
	char *tmpcat = NULL;
	while((direntp = readdir(dirp)) != NULL){
		if (direntp->d_name[0] != '.') { 
			catlen = strlen(dirname) + strlen(direntp->d_name) + 1;//1 extra for '/'
			tmpcat = (char*) calloc(catlen,  sizeof(char*));
			buildPath(tmpcat, dirname, direntp->d_name);
			if(direntp->d_type == DT_DIR){
				size = traverseDir(tmpcat, count +1, inodeSet);
				cumSize += size;
				printf("%d %s\n",size, tmpcat);
			}
			else{ 
				cumSize += sizeOfFile(tmpcat, inodeSet);
			}
		}
		else if (direntp->d_name[1] == '\0'){//if we are at .
			cumSize += DIRSIZE;//du seems to do this
		}
	}
	free(tmpcat);
	if(!closedir(dirp)){
		perror("failed to close a dir");
	}
	return cumSize;
}

//returns 1 if the fiven inode is already counted, 0 otherwise
int contains(uintmax_t inode, arrSet* inodeSet){
	for(int i = 0; i < inodeSet->size; i++){
		if(inode == inodeSet->array[i]){
			return 1;
		}
	}
	return 0;

}
//Takes a directory string an returns its size. The size of files already accounted for is 0
int sizeOfFile(char* file, arrSet* inodeSet){
	struct stat statBuf;
	if(!lstat(file, &statBuf)){
		perror("lstat failed");
	}
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
