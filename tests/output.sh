#!/bin/bash

SCRIPTPATH="$(cd "$(dirname ${BASH_SOURCE[0]})" && pwd)"

sudo "${SCRIPTPATH}/output" -p 26:19:13:6:5 -r 3 -d 0 -w 20000 -e 20000 -s 200

## RA Tracking: Not Perfect but Close
##
## Update to match the old tracking (Moon on Jan 10 2020) after cleaning up
## the timing.
##
#sudo "${SCRIPTPATH}/output" -p 26:19:13:6:5 -r 3 -d 0 -w 572 -e 12475 -s 0
