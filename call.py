#!/usr/bin/python

# A python script the runs the order searcher n number of times depending
# on what the argument was when call.py was called.
# This script was done with the help of Matt Dumbford, thanks buddy

from subprocess import call
import sys

try:
	sys.argv[1]
except IndexError:
	sys.argv.append("1")

for i in range(int(sys.argv[1])):
	for x in range(20):
		call("./orderSearcher.out VRUPL_Logo.raw " + str(i + 1), shell=True)
