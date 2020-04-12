#!/bin/bash

SCRIPTPATH="$(cd "$(dirname ${BASH_SOURCE[0]})" && pwd)"

sudo "${SCRIPTPATH}/rate" -p 26:19:13:6:5 -a 0 -d 0 -r 15.0

## RA Tracking: Not Perfect but Close
##
## Update to match the old tracking (Moon on Jan 10 2020) after cleaning up
## the timing.
##
#sudo "${SCRIPTPATH}/rate" -p 26:19:13:6:5 -a 0 -d 0 -r 15.0
