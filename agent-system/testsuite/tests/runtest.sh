#!/bin/bash

unset Y2DEBUG
unset Y2DEBUGGER
export Y2DEBUGSHELL=1

(./runag_system -l - $1 >$2) 2>&1 | fgrep -v " <0> " | grep -v "^$" | sed 's/^....-..-.. ..:..:.. [^)]*) //g' > $3
