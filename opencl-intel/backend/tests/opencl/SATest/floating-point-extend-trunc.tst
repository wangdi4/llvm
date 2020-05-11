; RUN: SATest -BUILD --config=%s.cfg -cpuarch="corei7" --dump-llvm-file - | FileCheck %s

; CHECK: Test program was successfully built.
