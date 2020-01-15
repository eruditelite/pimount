#!/bin/bash

SCRIPTPATH="$(cd "$(dirname ${BASH_SOURCE[0]})" && pwd)"

sudo "${SCRIPTPATH}/output" -p 26:19:13:6:5 -r 3 -d 0 -w 20000 -e 20000 -s 200

## RA Tracking: Not Perfect but Close
#sudo "${SCRIPTPATH}/output" -p 26:19:13:6:5 -r 3 -d 0 -w 500 -e 12400 -s 0
