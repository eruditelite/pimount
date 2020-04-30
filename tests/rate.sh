#!/bin/bash

SCRIPTPATH="$(cd "$(dirname ${BASH_SOURCE[0]})" && pwd)"

if [ "$1" = "trace" ]
then
    pushd "${SCRIPTPATH}" >/dev/null
    sudo gdb -quiet -command=rate.gdbtrace ./rate >rate.log
    popd >/dev/null
else
    sudo "${SCRIPTPATH}/rate" -a ra -d 2000 -r 15
fi

## RA Tracking: Not Perfect but Close
##
## Update to match the old tracking (Moon on Jan 10 2020) after cleaning up
## the timing.
##
#sudo "${SCRIPTPATH}/rate" -a ra -d pos -u 0 -r 15
