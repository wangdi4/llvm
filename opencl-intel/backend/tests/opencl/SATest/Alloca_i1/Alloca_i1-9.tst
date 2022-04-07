; Check that the compiler can work with i1 allocas

; RUN: SATest -BUILD -tsize=1 -config=%s.cfg -dump-llvm-file - | FileCheck %s
; RUN: SATest -BUILD -tsize=4 -config=%s.cfg -dump-llvm-file - | FileCheck %s

; CHECK: define void @set_false
; CHECK: Test program was successfully built.
