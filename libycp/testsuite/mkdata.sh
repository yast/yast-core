#! /bin/bash
# mkdata make test data
#
(../runycp <$1.ycp >$1.out) 2>&1 | fgrep -v " <0> " | sed 's/^....-..-.. ..:..:.. [^:]*://g' > $1.err

