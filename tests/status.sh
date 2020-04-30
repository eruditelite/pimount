#!/bin/bash

SCRIPTPATH="$(cd "$(dirname ${BASH_SOURCE[0]})" && pwd)"

if [ "$1" = "trace" ]
then
    pushd "${SCRIPTPATH}" >/dev/null
    sudo gdb -quiet -command=status.gdbtrace ./status >rate.log
    popd >/dev/null
else
    sudo "${SCRIPTPATH}/status"
fi
