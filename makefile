#cs 385 HW3
CC=gcc

all: orderSearcher.out

orderSearcher.out: orderSearcher.c
	gcc orderSearcher.c -pthread -lm -o orderSearcher.out
