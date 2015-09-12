#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// strtol with some error handling hopefully
int strtoint(char* str){
	char* end;
	int ret = strtol(str, &end, 10);//TODO this error is not being returned
	if(!end) //conversion error
		fprintf(stderr, "ERROR\n");
		//puts("ERROR"); //perror("string could not be converted to int");
	return ret;
}

void printArray(int* lifeArray, int row, int col, int gen){//TODO should we pass gen just for this?
	char* outStr = (char *)malloc(sizeof(char)*(row)*(col+1));
	int index = 0;
	for(int i = 1; i < col-1; i++){
		for(int j = 1; j < row-1; j++){
			if(!lifeArray[(row*i)+j])//index should never be called on lifeArray bacuse of the \n
				outStr[index++] = '-';
			else
				outStr[index++] = '*';
		}
		outStr[index++] = '\n';
	}
	printf("Generation %d:\n%s",gen, outStr);//TODO do not print borders
}

//create the Life array, read in the init file and bring the appropriate cells to life
int* createLife(int row, int col){
	int* lifeArray = (int *)calloc(row*col, sizeof(int)); //2d is an illusion.
	//TODO Read in the file here and turn on the appropriate things
	return lifeArray;
}

void iterateLife(int* lifeArray, int row, int col, int gen){
	for(int currGen = 0; currGen < gen; currGen++){
		printArray(lifeArray, row, col, currGen);
	}

}
//deleteLife()

int main(int argc,char* argv[]){
	int row = 12; //+2 for border
	int col = 12;
	char* filename = "life";
	int gen = 10;

	printf("argc: %d\n", argc);
	if(argc > 1)
		row = 2 + strtoint(argv[1]);
	if(argc > 2)
		col = 2 + strtoint(argv[2]);
	if(argc > 3)
		filename = argv[3];
	if(argc > 4)
		gen = strtoint(argv[4]);

//	printf("row: %d col: %d name: %s gen: %d \n", rows, cols, filename, gen);
	int* lifeArray = createLife(row, col);
	iterateLife(lifeArray, row, col, gen);
//	printArray(lifeArray, rows, cols);
	
	return 0;
}
