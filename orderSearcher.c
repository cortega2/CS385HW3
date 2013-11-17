#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>

#define SEMNUM 7

//globals
void * thread_function(void * arg);
sem_t sems[7];

unsigned int highestAvg = 0;
unsigned int lowestAvg = 255;
unsigned int maxChange = 0;
unsigned long maxSumChange = 0;
double lowStdDev = 255.0;
double lowStdChangeDev = 255.0;
int range[2] = {255, 0};


struct threadArgs{
	int id;
	long dataSize;
	int *data;
};

int main(int argc, char * argv[]){
	//simple error check
	printf("NAME: Carlos Ortega  NETID: corteg20\n");

	if(argc <=2){
		printf("Not enought arguments\n");
		return -1;
	}

	//file handling
	int numThreads = atoi(argv[2]);
	FILE *file;
	long flength;
	unsigned char *fileData;

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
	//end of file handing


	//create 2darray that will be used by the threads
	unsigned int *intData[numThreads]; 
	int x;
	for(x=0; x<numThreads; x++){
		//if not evenly divisible
		if(x== numThreads - 1)
			intData[x] = (unsigned int *) malloc((flength/numThreads + flength%numThreads)*sizeof(unsigned int));
		else
			intData[x] = (unsigned int *) malloc((flength/numThreads)*sizeof(unsigned int));
	}
    
	//populate int 2darray with values from the file which are in the fileData array and set
	//pointer to the array in the struct that will be passed to each thread
	struct threadArgs toTheThreads[numThreads];
	int counter = 0	;
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

	//initialize semaphores
	for(x = 0; x < SEMNUM; x++ ){
		res = sem_init(&sems[x], 0, 1);
		if(res != 0){
			perror("semaphore creation error");
			exit(EXIT_FAILURE);
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
	}	

	//wait for threads
	printf("Waiting for the thread to finish\n");
	for(x=numThreads-1; x >= 0; x--){

		res = pthread_join(many_threads[x], &thread_result);
		if( res != 0){
			perror("Thread joining failed");
			exit(EXIT_FAILURE);
		}
	}

	printf("Highest Average: %d\n", highestAvg);	
	printf("Lowest Average: %d\n", lowestAvg);
	printf("Highest Range: %d\nLowest Range: %d\n", range[1], range[0]);
	printf("Max change: %d\n", maxChange);
	printf("Max Sum Change: %d\n", maxSumChange);
	printf("Min Standard Dev: %f\n", lowStdDev);
	printf("Min Std Dev of Data Change: %f\n", lowStdChangeDev);
	printf("Done! \n");

	for(x = 0; x < SEMNUM; x++)
		sem_destroy(&sems[x]);

	exit(EXIT_SUCCESS);
}

void * thread_function(void * arg){
	struct threadArgs vars = *(struct threadArgs *) arg;
	
	//calculate average and range and change
	long size = vars.dataSize;
	long average = 0;
	long x;
	int lowest = 255;
	int highest = 0;
	int Localrange = 0;
	int change = 0;
	int changeSum = 0;
	double LocalStdDev = 0;
	int diff = 0;
		
	for(x = 0; x<size; x++){
		if(vars.data[x] < lowest)
			lowest = vars.data[x];
		if(vars.data[x] > highest)
			highest = vars.data[x];
		average = average + vars.data[x];

		if(x!= size - 1){
			int absolute = abs(vars.data[x] - vars.data[x+1]);
			changeSum = changeSum + absolute;
			if( absolute > change)
				change = absolute;
		}
	}
	average = average/size;
	Localrange = highest - lowest;

	//update highest average
	sem_wait(&sems[0]);
	if(average > highestAvg)
	highestAvg = average;
	sem_post(&sems[0]);

	//update lowest average
	sem_wait(&sems[1]);
	if(average < lowestAvg)
		lowestAvg = average;
	sem_post(&sems[1]);

	//update ranges
	sem_wait(&sems[2]);
	if(Localrange < range[0])
		range[0] = Localrange;
	if(Localrange > range[1])
		range[1] = Localrange;
	sem_post(&sems[2]);

	//update max changes
	sem_wait(&sems[3]);
	if(change > maxChange)
		maxChange = change;
	sem_post(&sems[3]);

	//update maxSumChange
	sem_wait(&sems[4]);
	if(changeSum > maxSumChange)
		maxSumChange = changeSum;
	sem_post(&sems[4]);

	//find std dev
	for(x = 0; x < size; x++)
		diff = diff + (vars.data[x] - average) * (vars.data[x] - average);

	LocalStdDev = sqrt((double)diff/size);
	
	sem_wait(&sems[5]);
	if(LocalStdDev < lowStdDev)
		lowStdDev = LocalStdDev;
	sem_post(&sems[5]);

	//find std dev of change of data
	double LocalStdChangeDev = 0;
	double changeDiff = 0.0;
	double absSumAvg = 0.0;
	for(x =0; x < size-1; x ++)
		absSumAvg = absSumAvg + abs(vars.data[x] - vars.data[x+1]);
	absSumAvg = (double)absSumAvg / (size-1);

	for(x = 0; x < size -1 ;x++){
		int absolute = abs(vars.data[x] - vars.data[x+1]);
		changeDiff = changeDiff + (absolute - absSumAvg) * (absolute- absSumAvg);
	}
	LocalStdChangeDev = sqrt(changeDiff / (size-1));
	sem_wait(&sems[6]);
	if(LocalStdChangeDev < lowStdChangeDev)
		lowStdChangeDev = LocalStdChangeDev;
	sem_post(&sems[6]);


	pthread_exit("Thank you for the cpu time");
}
