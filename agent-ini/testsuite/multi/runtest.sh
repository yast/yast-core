#!/bin/bash

#export Y2DEBUG=1
# in the NI, Y2DEBUG is awfully verbose.
unset Y2DEBUGGER

IN_FILE=${1%.*}".in.*.test"
rm -f $IN_FILE 2> /dev/null

IN_FILE=${1%.*}".in.[0-9]"
for I in $IN_FILE; do
    rm -f $I.test 2> /dev/null
    cp $I $I.test 2> /dev/null
done

(./runag_ini -l - $1 >$2) 2>&1 | grep -v "^$" | sed '-e s/^....-..-.. ..:..:.. [^)]*) //g' -e 's/:[0-9]\+ /:XXX /' > $3
##(./runag_ini -l - $1 >$2) 2>&1 | fgrep -v " <0> " | grep -v "^$" | sed 's/^....-..-.. ..:..:.. [^)]*) //g' > $3

IN_FILE=${1%.*}".in.[0-9].test"
for I in $IN_FILE; do
    echo $I " -------------------------------" >> $2 2> /dev/null
    cat $I >> $2 2> /dev/null
done

