#!/usr/bin/env python3

import os
import sys
import getopt
from matplotlib import pyplot
import numpy
#import scipy

##################
## Parse Inputs ##
##################

def usage():
    print("USAGE: %s [-h|--help] -i|--input <filename>"
          % (os.path.basename(sys.argv[0])))

argv = sys.argv[1:]
try:
    opts, args = getopt.getopt(argv, 'hi:', ['help', 'input='])
except getopt.GetoptError as error:
    print(error)
    usage()
    sys.exit(2)

# 'opts' is an array containing the parsed arguments
# 'args' is everything else.

# If -h|--help, display useage and exit.

if [n for n, v in enumerate(opts) if v[0] == '-h' or v[0] == '--help']:
    usage()
    sys.exit(0)

# Make sure -i|--input was given...

if not [n for n, v in enumerate(opts) if v[0] == '-i' or v[0] == '--input']:
    print("-i|--input <filename> is REQUIRED")
    usage()
    sys.exit(2)

###########################
## Do Something "Useful" ##
###########################

inputName = opts[[v for v, n in enumerate(opts)
                  if n[0] == '-i' or n[0] == '--input'][0]][1]
inputFd = open(os.path.abspath(inputName), "r")
input = inputFd.readlines()[3:]

time = []
v1 = []
v2 = []

for line in input:
    values = line.split(", ")
    time.append(float(values[0]))
    v1.append(float(values[1]) - float(values[2]))
    v2.append(float(values[3]) - float(values[4]))

pyplot.plot(time, v1)
pyplot.plot(time, v2)
pyplot.xlabel("Seconds")
pyplot.ylabel("Volts")
pyplot.legend(['v1', 'v2'])
pyplot.show()

##########
## Done ##
##########

sys.exit(0)
