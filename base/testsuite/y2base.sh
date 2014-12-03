#!/bin/sh
ROOT=../..
LIBS=src/.libs
AG_ANY_DIR=$ROOT/agent-any/$LIBS
# The agent is looked for not in $Y2DIR but in $Y2DIR/plugin so create 'plugin'
ln -snf . $AG_ANY_DIR/plugin
export Y2DIR=$AG_ANY_DIR
export LD_LIBRARY_PATH="\
$ROOT/liby2util-r/$LIBS:\
$ROOT/libycp/$LIBS:\
$ROOT/liby2/$LIBS:\
$ROOT/libscr/$LIBS:\
$ROOT/wfm/$LIBS:\
$ROOT/scr/$LIBS\
"
# ignore incompatible ruby-bindings while developing
export Y2DISABLELANGUAGEPLUGINS=1
../$LIBS/y2base -l ${1%.*}.log ./$1 -S "($(pwd))" testsuite
