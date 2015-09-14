#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// strtol with some error handling hopefully
int strtoint(char* str){
	char* end;
	int ret = strtol(str, &end, 10);//TODO this error is not being returned
	if(str == end){ //conversion error
		fprintf(stderr, "input could not be parsed\n");
		exit(1);
	}
	return ret;
}

//pretty-print the world
void printArray(int* lifeArray, int row, int col, int currGen){
	char* outStr = (char *)malloc(sizeof(char)*(row)*(col+1));
	int index = 0;
	for(int i = 1; i < col-1; i++){
		for(int j = 1; j < row-1; j++){
			if(!lifeArray[(row*i)+j])
				outStr[index++] = '-';
			else
				outStr[index++] = '*';
		}
		outStr[index++] = '\n';
	}
	printf("Generation %d:\n%s",currGen, outStr);
}

//print used in debugging to check the raw values of the cells
void printRawArray(int* lifeArray, int size){
	for(int i = 0; i < size; i++)
		printf("%d",lifeArray[i]);
	printf("\n");
}

//create the Life array, read in the init file and bring the appropriate cells to life
int* createLife(int row, int col, char* filename){ //if the input file assumes the world is bigger than it actually is, bad things happen.
	int* lifeArray = (int *)calloc(row*col, sizeof(int)); //2d is an illusion.
	FILE * file;
	file = fopen(filename, "r");
	if (file == NULL) {
		fprintf(stderr, "Can't open input file\n");
		exit(1);
	}
	char tmp;
	int currRow = 1;
	int currCol = 1;
	while ((tmp = fgetc(file)) != EOF){
		if('*' == tmp){//living
			lifeArray[(row*currCol)+currRow] = 1;
			currRow++;
		}
		else if (' ' == tmp) //dead
			currRow++;
		else { //newline
			currRow = 1;
			currCol++;
		}
	}
	return lifeArray;
}

//get the total number of living neighbors for a cell
int countLivingNeighbors(int* lifeArray, int row, int index){
	return lifeArray[index-row-1]+ lifeArray[index-row]+ lifeArray[index-row+1]+ lifeArray[index-1]+
	lifeArray[index+1]+ lifeArray[index+row-1]+ lifeArray[index+row]+ lifeArray[index+row+1];
}

//update the world gen number of times
void iterateLife(int* lifeArray, int row, int col, int gen){
	printArray(lifeArray, row, col, 0);
	for(int currGen = 1; currGen <= gen; currGen++){
		int* newLifeArray = (int *)calloc(row*col, sizeof(int)); //2d is an illusion.
		int index = 0;
		int neigh = 0;
		for(int i = 1; i < col-1; i++){
			for(int j = 1; j < row-1; j++){
				index = (i*row)+j;
				neigh = countLivingNeighbors(lifeArray, row, index);
				if(lifeArray[index]){//it is alive
					if(neigh < 2 || neigh > 3)
						newLifeArray[index] = 0;
					else
						newLifeArray[index] = 1;
				}
				else if (neigh == 3){
					newLifeArray[index] = 1;
				}
			}
		}
		free(lifeArray);
		lifeArray = newLifeArray;
		printArray(lifeArray, row, col, currGen);
	}
	free(lifeArray);
}

int main(int argc,char* argv[]){
	int row = 12; //+2 for border
	int col = 12;
	char* filename = "life";//TODO what if bigger
	int gen = 10;

	if(argc > 1)
		row = 2 + strtoint(argv[1]);
	if(argc > 2)
		col = 2 + strtoint(argv[2]);
	if(argc > 3)
		filename = argv[3];
	if(argc > 4)
		gen = strtoint(argv[4]);

	int* lifeArray = createLife(row, col, filename);
	iterateLife(lifeArray, row, col, gen);
	return 0;
}
