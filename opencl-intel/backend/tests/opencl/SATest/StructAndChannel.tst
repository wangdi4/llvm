; RUN: SATest -BUILD --config=%s.cfg --dump-llvm-file - | FileCheck %s

; CHECK: Test program was successfully built.
