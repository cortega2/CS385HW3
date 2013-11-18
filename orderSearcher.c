#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/msg.h>
#include <math.h>

#define MSG_RW 0600 | IPC_CREAT
#define SEMNUM 7

//globals
void * thread_function(void * arg);
void * thread_visual(void  * arg);
const long int BYE = 1;
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
	int msgID;
};

struct message{
        long int type;
        int ID;
        char symbols[81];
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

	//create message queue
    int msgID = msgget(IPC_PRIVATE, MSG_RW);
    if(msgID == -1){
   	    printf("msgget error\n");
        return -1;
    }
    else
  		printf("Created a MSG queue, ID: %d\n", msgID);

  	//create visualization 2d array and struct for thread that will handle messages
  	struct threadArgs messageArgs;
  	messageArgs.id = numThreads;
  	messageArgs.msgID = msgID;

	//Create threads
  	pthread_t messageThread;
  	res = pthread_create(&(messageThread), NULL, thread_visual, (void *)&messageArgs);
		if(res != 0){
			perror("Thread creation failed");
			exit(EXIT_FAILURE);
		}

	for(x=0; x<numThreads; x++){
		toTheThreads[x].id = x;
		toTheThreads[x].msgID = msgID;

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
	res = pthread_join(messageThread, &thread_result);
		if( res != 0){
			perror("Thread joining failed");
			exit(EXIT_FAILURE);
		}

	printf("Highest Average: %d\n", highestAvg);	
	printf("Lowest Average: %d\n", lowestAvg);
	printf("Highest Range: %d\nLowest Range: %d\n", range[1], range[0]);
	printf("Max change: %d\n", maxChange);
	printf("Max Sum Change: %d\n", maxSumChange);
	printf("Min Standard Dev: %f\n", lowStdDev);
	printf("Min Std Dev of Data Change: %f\n", lowStdChangeDev);
	printf("Done! \n");

	//clean up
	for(x = 0; x < SEMNUM; x++)
		sem_destroy(&sems[x]);
	msgctl(msgID, IPC_RMID, NULL);

	exit(EXIT_SUCCESS);
}

void *thread_visual(void *arg){
	struct threadArgs vars = *(struct threadArgs *) arg;
	char visuals[vars.id][81];

	int count = vars.id;
	while(count){
		struct message recieve;
		if(msgrcv(vars.msgID, &recieve, sizeof(struct message) - sizeof(long int), 0, 0) < 0){
         	perror("ERROR RECIEVING START MESSAGE\n");
           	exit(EXIT_FAILURE);
       	}

       	if(recieve.type == BYE){
            printf("End message from THREAD[%d]\n", recieve.ID);
            printf("%s\n", recieve.symbols);
            strcpy(visuals[recieve.ID], recieve.symbols);
        	count--;
        }

	}

	int x;
	printf("\n\nHow the data looks together:\n");
	for(x = 0; x < vars.id; x++){
		int q;
		for(q = 0; q<81; q++)
			printf("%c",visuals[x][q]);
		printf("\n");
	}
	printf("\n");
	pthread_exit("Thank you for the cpu time");
}

void *thread_function(void *arg){
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
	if(highestAvg < 255){
		sem_wait(&sems[0]);
		if(average > highestAvg)
		highestAvg = average;
		sem_post(&sems[0]);
	}
	
	//update lowest average
	if(lowestAvg > 0){
		sem_wait(&sems[1]);
		if(average < lowestAvg)
			lowestAvg = average;
		sem_post(&sems[1]);
	}

	//update ranges
	if(range[0] > 0 || range[1] < 255){
		sem_wait(&sems[2]);
		if(Localrange < range[0])
			range[0] = Localrange;
		if(Localrange > range[1])
			range[1] = Localrange;
		sem_post(&sems[2]);
	}

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
	if(lowStdDev > 0){
		for(x = 0; x < size; x++)
			diff = diff + (vars.data[x] - average) * (vars.data[x] - average);

		LocalStdDev = sqrt((double)diff/size);
		
		sem_wait(&sems[5]);
		if(LocalStdDev < lowStdDev)
			lowStdDev = LocalStdDev;
		sem_post(&sems[5]);
	}
	
	//find std dev of change of data
	if(lowStdChangeDev > 0 ){
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
	}

	//try and visualize
	struct message toSend;
	toSend.type = BYE;
	toSend.ID = vars.id;

	unsigned int avgs[80];
	char symbols[81];
	long chunkSize = size/80;
	int curPos = 0;
	for(x = 0; x < 80; x++){
		int q;
		unsigned long sum = 0;
		for(q = 0; q < chunkSize; q++){
			sum = sum + vars.data[curPos];
			curPos++;
		}

		avgs[x] = sum/chunkSize;
		
		if(avgs[x] >= 0 && avgs[x] < 25)
			symbols[x] = 'w';
		else if(avgs[x] >= 25 && avgs[x] < 50)
			symbols[x] = 'x';
		else if(avgs[x] >= 50 && avgs[x] < 75)
			symbols[x] = 'z';
		else if(avgs[x] >= 75 && avgs[x] < 100)
			symbols[x] = '@';
		else if(avgs[x] >= 100 && avgs[x] < 125)
			symbols[x] = '#';
		else if(avgs[x] >= 125 && avgs[x] < 150)
			symbols[x] = '$';
		else if(avgs[x] >= 150 && avgs[x] < 175)
			symbols[x] = '%';
		else if(avgs[x] >= 175 && avgs[x] < 200)
			symbols[x] = '&';
		else if(avgs[x] >= 200 && avgs[x] < 225)
			symbols[x] = '*';
		else
			symbols[x] = '=';
	}
	symbols[81] = '\0';
	strcpy(toSend.symbols, symbols);
	if(msgsnd(vars.msgID, &toSend, sizeof(struct message) - sizeof(long int), IPC_NOWAIT) < 0){
       	perror("ERROR SENDING START MESSAGE!\n");
       	exit(EXIT_FAILURE);
    }

	//printf("%s\n", symbols);

	pthread_exit("Thank you for the cpu time");
}
