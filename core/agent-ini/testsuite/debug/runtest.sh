#!/bin/bash

export Y2DEBUG=1
unset Y2DEBUGGER

IN_FILE=${1%.*}".in"
rm -f "$IN_FILE.test" 2> /dev/null
cp $IN_FILE "$IN_FILE.test" 2> /dev/null

(./runag_ini -l - $1 >$2) 2>&1 | grep -v "^$" \
    | grep -E '^....-..-.. ..:..:.. (.*\[agent-ini\]|<[1-5]>)' \
    | sed '-e s/^....-..-.. ..:..:.. [^)]*) //g' -e 's/:[0-9]\+ /:XXX /' > $3
##(./runag_ini -l - $1 >$2) 2>&1 | fgrep -v " <0> " | grep -v "^$" | sed 's/^....-..-.. ..:..:.. [^)]*) //g' > $3

cat "$IN_FILE.test" >> $2 2> /dev/null

