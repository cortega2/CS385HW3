#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

void * thread_function(void * arg);
char message[] = "hello";

struct threadArgs{
	int id;
	int *data;
};

int main(int argc, char * argv[]){
	//simple error check
	if(argc <=2){
		printf("Not enought arguments\n");
		return -1;
	}

	//create variables for threads
	int numThreads = atoi(argv[2]);
	int res;
	pthread_t many_threads[numThreads];
	struct threadArgs toTheThreads[numThreads];
	void * thread_result;

	//Read file
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

	/*
	long q;
	for(q=0; q<flength; q++){
		printf("%c",(fileData)[q]);
	}
	*/
	
	////////////////////////////////////////////////////////////////////test code<<<<<<<<<<<<<<<<<<<<<<
	int foo = (int) (fileData)[0];
	char poo = (fileData)[0];
	printf("int %d: char: %c \n", foo, poo);
	printf("%d\n",(int)sizeof(char));
	printf("%d\n",(int)sizeof(int));
	int ddd = flength/numThreads;
	printf("%d\n", ddd);
	//end of test code<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

	//create 2darray that will be used by the threads 
	int *intData[numThreads]; 
	int x;
	for(x=0; x<numThreads; x++){
		if(x== numThreads - 1)
			intData[x] = (int *) malloc(flength/numThreads + flength%numThreads);
		else
			intData[x] = (int *) malloc(flength/numThreads);
	}
    
	//populate int 2darray with values from the file which are in the fileData array
	int counter = 0;
	for(x=0; x<numThreads; x++){
		int limit;
		int y;
		if(x == numThreads - 1)
			limit = flength/numThreads + flength%numThreads;
		else
			limit = flength/numThreads;

		printf("LIMIT: %d\n", limit);
		for(y=0; y<limit; y++){
			intData[x][y] = (fileData)[counter];
			printf("DATA IN [%d][%d] = [%d]\n", x, y, intData[x][y]);
			printf("counter: %d\n", counter);
			counter ++;
		}
	}

	//Create threads
	int temp =3;
	for(x=0; x<numThreads; x++){
		toTheThreads[x].id = x;
		res = pthread_create(&(many_threads[x]), NULL, thread_function, (void *)&toTheThreads[x]);
		if(res != 0){
			perror("Thread creation failed");
			exit(EXIT_FAILURE);
		}
		sleep(1);
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
	//printf();
	struct threadArgs vars = *(struct threadArgs *) arg;
	printf("thread_function is running. Argument was %d\n", vars.id);
	sleep(3);
	//strcpy(message, "Bye!");
	pthread_exit("Thank you for the cpu time");
}
