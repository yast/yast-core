#!/bin/sh
T=../..                         # Top dir
L=src/.libs                     # Lib subdir
AG_ANY_DIR=$T/agent-any/$L
ln -snf . $AG_ANY_DIR/plugin
export Y2DIR=$AG_ANY_DIR
export LD_LIBRARY_PATH=$T/liby2util-r/$L:$T/libycp/$L:$T/liby2/$L:$T/libscr/$L:$T/wfm/$L:$T/scr/$L
# ignore incompatible ruby-bindings while developing
export Y2DISABLELANGUAGEPLUGINS=1
../$L/y2base -l ${1%.*}.log ./$1 -S "($(pwd))" testsuite
