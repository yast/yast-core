#!/bin/bash

unset LANG
unset LC_MESSAGES
unset LC_ALL
unset LANGUAGE

unset Y2DEBUG
unset Y2DEBUGGER

(./runwfm -l - $1 >$2) 2>&1 | fgrep -v " <0> " | grep -v "^$" | sed -e 's/^....-..-.. ..:..:.. [^)]*) //g' -e 's/):[0-9]\+ /):XXX /' > $3
