#!/bin/bash

unset Y2DEBUG
unset Y2DEBUGALL
unset Y2DEBUGGER

export EF_ALLOW_MALLOC_0=1
export Y2SILENTSEARCH=1
# for float::tolstring
export LC_NUMERIC=cs_CZ.UTF8

(./runycp -l - -I tests/Include -M tests/Module $1 >$2) 2>&1 | fgrep -v 'Electric Fence' | fgrep -v " <0> " | grep -v "^$" | sed 's/^....-..-.. ..:..:.. [^)]*) //g' > $3
exit 0
