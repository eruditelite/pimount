#!/bin/bash

SCRIPTPATH="$(cd "$(dirname ${BASH_SOURCE[0]})" && pwd)"

if [ "$1" = "trace" ]
then
    pushd "${SCRIPTPATH}" >/dev/null
    sudo gdb -quiet -command=threads.gdbtrace ./threads >threads.log
    popd >/dev/null
else
    sudo "${SCRIPTPATH}/threads" -a ra -r 15
fi
