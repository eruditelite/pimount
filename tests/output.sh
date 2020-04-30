#!/bin/bash

SCRIPTPATH="$(cd "$(dirname ${BASH_SOURCE[0]})" && pwd)"

if [ "$1" = "trace" ]
then
    pushd "${SCRIPTPATH}" >/dev/null
    sudo gdb -quiet -command=output.gdbtrace ./output >output.log
    popd >/dev/null
else
    sudo "${SCRIPTPATH}/output" -a ra -r half -d cw -w 10000 -e 20000 -s 10
fi

## RA Tracking: Not Perfect but Close
##
## Update to match the old tracking (Moon on Jan 10 2020) after cleaning up
## the timing.
##
#sudo "${SCRIPTPATH}/output" -a ra -r half -d cw -w 572 -e 12475 -s 0

##if 0
#static struct speed ra_speeds[] = {
#	{1, A4988_RES_HALF, A4988_DIR_CW, 500, 4000},
#	{1, A4988_RES_HALF, A4988_DIR_CW, 500, 6000},
#	{1, A4988_RES_HALF, A4988_DIR_CW, 500, 8000},
#	{1, A4988_RES_HALF, A4988_DIR_CW, 500, 10000},
#	{1, A4988_RES_EIGHTH, A4988_DIR_CW, 500, 12000}, /* Earth's Rotation */
#	{0, A4988_RES_HALF, A4988_DIR_CCW, 500, 10000},  /* Just stop... */
#	{1, A4988_RES_HALF, A4988_DIR_CCW, 500, 8000},
#	{1, A4988_RES_HALF, A4988_DIR_CCW, 500, 6000},
#	{1, A4988_RES_HALF, A4988_DIR_CCW, 500, 4000},
#};
#
##define RA_SPEED_DEFAULT 4
##define RA_SPEED_MAX 8
#
#static struct speed dec_speeds[] = {
#	{1, A4988_RES_HALF, A4988_DIR_CCW, 500, 4000},
#	{1, A4988_RES_HALF, A4988_DIR_CCW, 500, 6000},
#	{1, A4988_RES_HALF, A4988_DIR_CCW, 500, 8000},
#	{1, A4988_RES_HALF, A4988_DIR_CCW, 500, 10000},
#	{0, A4988_RES_HALF, A4988_DIR_CCW, 500, 0}, /* OFF */
#	{1, A4988_RES_HALF, A4988_DIR_CW, 500, 100000},
#	{1, A4988_RES_HALF, A4988_DIR_CW, 500, 8000},
#	{1, A4988_RES_HALF, A4988_DIR_CW, 500, 6000},
#	{1, A4988_RES_HALF, A4988_DIR_CW, 500, 4000},
#};
#
##define DEC_SPEED_DEFAULT 4
##define DEC_SPEED_MAX 8
##endif
