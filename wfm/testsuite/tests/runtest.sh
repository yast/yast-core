#!/bin/bash

unset LANG
unset LC_ALL

unset Y2DEBUG
unset Y2DEBUGGER

(./runwfm -l - $1 >$2) 2>&1 | fgrep -v " <0> " | grep -v "^$" | sed 's/^....-..-.. ..:..:.. [^)]*) //g' > $3
