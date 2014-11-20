#!/bin/sh
if false; then
    export Y2DEBUG=1
    STRACE="strace -ostrace.log -efile -f"
fi
R=$RPM_BUILD_ROOT
export LD_LIBRARY_PATH=$R/usr/lib64:$R/usr/lib64/YaST2/plugin
export Y2DIR=$R/usr/share/YaST2:$R/usr/lib64/YaST2
$STRACE $R/usr/lib/YaST2/bin/y2base -l ${1%.*}.log ./$1 testsuite
