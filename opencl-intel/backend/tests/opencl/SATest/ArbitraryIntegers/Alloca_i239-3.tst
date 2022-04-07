; Check that the compiler can work with arbitrary integer allocas

; RUN: SATest -BUILD -tsize=1 -config=%s.cfg -dump-llvm-file - | FileCheck %s
; RUN: SATest -BUILD -tsize=4 -config=%s.cfg -dump-llvm-file - | FileCheck %s

; CHECK: define void @print_zero
; CHECK: Test program was successfully built.
