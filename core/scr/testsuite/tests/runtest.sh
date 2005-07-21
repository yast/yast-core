#!/bin/bash

unset Y2DEBUG
unset Y2DEBUGGER

(./runscr -l - $1 >$2) 2>&1 | fgrep -v " <0> " | grep -v "^$" | sed 's/^....-..-.. ..:..:.. [^)]*) //g' > $3
