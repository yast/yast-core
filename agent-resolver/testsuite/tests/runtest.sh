#!/bin/bash

unset Y2DEBUG
unset Y2DEBUGALL
unset Y2DEBUGGER

IN_FILE=${1%.*}".in"
rm -f "$IN_FILE.test" 2>/dev/null
cp $IN_FILE "$IN_FILE.test" 2>/dev/null

SCR_FILE=${1%.*}".scr"
rm -f "$SCR_FILE" 2>/dev/null
echo -e ".\n\n\`ag_resolver(\n    \`ResolverAgent(\"$IN_FILE.test\")\n)\n" >"$SCR_FILE"

(./runresolver -l - "$1" >"$2") 2>&1 | fgrep -v " <0> " | grep -v "^$" | sed 's/^....-..-.. ..:..:.. [^)]*) //g' >$3

cat "$IN_FILE.test" >>"$2" 2>/dev/null
exit 0
