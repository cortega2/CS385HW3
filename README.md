CS385HW3
========

Searching for order among chaos using threads
---------------------------------------------
NAME: CARLOS ORTEGA		NETID: CORTEG20

Summary
-------


The purpose for this program is to try and find order in data files. The files that I used to test my program consisted of image files 
of different formats. Most of the image files however, were raw format. This was because jpeg and other similar image formats use 
compression techniques to lower the file size, meaning that for the purpose of the program so information may have been lost in these 
types of files. The program uses threads to analyze the file. 


The file is read in as unsigned chars and then that data is divided as evenly as possible between the threads. This way each thread 
gets a “chunk” of data that it can then analyze. Using branch and bound techniques, each thread will find various parameters such a 
standard deviation of the data. Each thread also tries to visualize the data.


The way that visualization works in my program is that the data that each thread receives is divided by 80 to create smaller “chunks” 
of data. Then the average of each chunk is calculated. Based on that average a symbol is assigned to that chunk. Each individual thread 
then sends the symbols to a printing thread using a message queue. The printing thread then puts the symbol in order by thread the 
thread ID so that the first thread’s symbols appear first when they are printed.

Files
-----
The following files should be included:
 -README.md
 -makefile
 -orderSearcher.c
 -call.py
 -VRUPL_Logo.raw
 -LICENSE

Running the Program
-------------------
To compile the program simply use the make file by typing in the command line make.

To execute the program do  <b>./orderSearcher.out [file name] [number of threads] </b>
Example: /orderSearcher.out VRUPL_Logo.raw 2

Note that for the program to run properly the number of threads that is entered has to be > 0

Using the Script
----------------
Included is the call.py script. This python script provides another way of running the program.
To run the script simply do:<b> ./call.py [number of threads]</b>

The script runs the program 20 times for each number of threads. This means that it will execute the program orderSearcher.out 20 times 
with 1 thread, then with 2, all the way up to N number of threads threads.

For the script I also used the VRUPL_Logo.raw file. This is hardcoded into the script but can be changed.





