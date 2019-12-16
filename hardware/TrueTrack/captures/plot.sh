#!/bin/bash

INPUT=$1
OUTPUT=$2
TFILE=$(mktemp /tmp/plot.sh.XXXXXXXXXXXXXXXX)
CFILE=$(mktemp /tmp/plot.sh.XXXXXXXXXXXXXXXX)

# remove the first two lines
tail +3 "${INPUT}" > $TFILE

# clean up
while read line
do
    SAVEIFS=$IFS
    IFS=', ' read -r -a values <<< "$line"
    IFS=$SAVEIFS
    A=$(echo "${values[1]} - ${values[2]}" | bc)
    B=$(echo "${values[3]} - ${values[4]}" | bc)
    echo "${values[0]} $A $B" >> "${OUTPUT}"
done <$TFILE

rm $TFILE

echo set title \"Stepper Driver\" >$CFILE
echo set xlabel \"Time \(s\)\" >>$CFILE
echo set ylabel \"Voltage\" >>$CFILE
echo set grid >>$CFILE
echo plot \
     \""${OUTPUT}"\" every ::0::2000 using 1:2, \
     \""${OUTPUT}"\" every ::0::2000 using 1:3 >>$CFILE
echo pause -1 >>$CFILE

echo "gnuplot commands..."
cat $CFILE

echo "Plotting, Press Return to End"
gnuplot $CFILE

rm $CFILE
