#!/bin/bash

unset Y2DEBUG
unset Y2DEBUGALL
unset Y2DEBUGGER

export LC_COLLATE=en_US

(./runycp -l - $1 >$2) 2>&1 | fgrep -v " <0> " | grep -v "^$" | sed 's/^....-..-.. ..:..:.. [^)]*) //g' > $3
