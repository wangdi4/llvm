; RUN: SATest -BUILD -config=%s.cfg -dump-llvm-file - | FileCheck %s

; This is kind of a naive test that checks if compiler can process bools. It is
; a helper part of series tests for i1 alloca.

; CHECK: Test program was successfully built.
