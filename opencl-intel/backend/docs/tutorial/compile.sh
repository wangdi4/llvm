#!/bin/bash

TMPFILE="/tmp/file.$RANDOM.bc"

LIBHELLO="../../build/Linux64/lib/LLVMHello.so"

function save_cfg {
    for f in cfg.*.dot; do
       dot $f -T jpeg -o $f_$1_$2.jpeg ;
       rm $f -f
    done
}

OPTFLAGS1="-unroll-threshold=512 -inline-threshold=4096 -inline -loop-rotate  \
          -loop-unroll  -std-compile-opts -mergereturn -loopsimplify -dot-cfg"

OPTFLAGS3=" -load=$LIBHELLO -hello -dot-cfg"

rm -f $TMPFILE
/bin/echo    $1 ":"
/bin/echo -n "Parsing "     && clang -emit-llvm -c $1 -o $TMPFILE
/bin/echo -n "Optimizing "  && opt $OPTFLAGS1 $TMPFILE -o $TMPFILE -f
/bin/echo -n "Plotting "    && save_cfg $1 1
/bin/echo -n "Optimizing "  && opt $OPTFLAGS3 $TMPFILE -o $TMPFILE -f
/bin/echo -n "Disas "       && llvm-dis $TMPFILE -o $1.output
/bin/echo -n "Plotting "    && save_cfg $1 2
/bin/echo "Done"
