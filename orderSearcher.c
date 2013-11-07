#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

void * thread_function(void * arg);
char message[] = "hello";

struct threadArgs{
	int id;
	long dataSize;
	int *data;
};

int main(int argc, char * argv[]){
	//simple error check
	if(argc <=2){
		printf("Not enought arguments\n");
		return -1;
	}

	//Read file
	int numThreads = atoi(argv[2]);
	FILE *file;
	long flength;
	char *fileData;

	file = fopen(argv[1], "rb");	
	if(!file){
		perror("Unable to open file");
		return -1;
	}

	fseek(file, 0, SEEK_END);
	flength = ftell(file);
	printf("file size: %ld\n", flength);
	
	fseek(file, 0, SEEK_SET);
	fileData = (char *) malloc(flength+1);
	fread(fileData, flength, 1, file);
	fclose(file);

	//create 2darray that will be used by the threads

	int *intData[numThreads]; 
	int x;
	for(x=0; x<numThreads; x++){
		if(x== numThreads - 1)
			intData[x] = (int *) malloc((flength/numThreads + flength%numThreads)*sizeof(int));			//if not evenly divisible
		else
			intData[x] = (int *) malloc((flength/numThreads)*sizeof(int));
	}
    
	//populate int 2darray with values from the file which are in the fileData array and set pointer to the array in the struct
	//that will be passed to each thread
	struct threadArgs toTheThreads[numThreads];
	int counter = 0;
	for(x=0; x<numThreads; x++){
		long limit;
		long y;
		if(x == numThreads - 1)
			limit = flength/numThreads + flength%numThreads;											//if not evenly divisible
		else
			limit = flength/numThreads;

		for(y=0; y<limit; y++){
			intData[x][y] = (fileData)[counter];
			counter ++;
		}
		toTheThreads[x].data = intData[x];
		toTheThreads[x].dataSize = limit;
	}
	
	//create variables for threads
	int res;
	pthread_t many_threads[numThreads];
	void * thread_result;

	//Create threads
	int temp =3;
	for(x=0; x<numThreads; x++){
		toTheThreads[x].id = x;
		res = pthread_create(&(many_threads[x]), NULL, thread_function, (void *)&toTheThreads[x]);
		if(res != 0){
			perror("Thread creation failed");
			exit(EXIT_FAILURE);
		}
	}	
	printf("Waiting for the thread to finish\n");
	for(x=numThreads-1; x >= 0; x--){

		res = pthread_join(many_threads[x], &thread_result);
		if( res != 0){
			perror("Thread joining failed");
			exit(EXIT_FAILURE);
		}
	}
	
	//printf("Thread joined, it returned %s\n", (char *)thread_result);
	printf("Done! \n");
	exit(EXIT_SUCCESS);
}

void * thread_function(void * arg){
	struct threadArgs vars = *(struct threadArgs *) arg;
	
	//calculate average
	long size = vars.dataSize;
	long average = 0;
	long x;
	for(x = 0; x<size; x++){
		average = average + vars.data[x];
	}
	average = average/size;
	printf("Thread ID[%d] Average: %ld\n", vars.id, average);

	pthread_exit("Thank you for the cpu time");
}
