#!/bin/bash

unset LANG
unset LC_ALL
unset Y2DEBUG
unset Y2DEBUGGER
export Y2DEBUGSHELL=1

# ignore also milestones (they are not constant and logged by liby2util - cannot be simply turned off)
(./run_ag_process -l - $1 >$2) 2>&1 | fgrep -v -e " <0> " -e " <1> " | grep -v "^$" | sed 's/^....-..-.. ..:..:.. [^)]*) //g' > $3
