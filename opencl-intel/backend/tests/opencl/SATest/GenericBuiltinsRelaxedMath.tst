;;; Test that builtins with pointers in generic address space can be replaced with "_rm" (relaxed math) versions correctly.

; RUN: SATest -BUILD --config=%s.cfg -tsize=0 --dump-llvm-file - | FileCheck %s
;
; CHECK: call fast nofpclass({{.*}}) float @_Z9sincos_rmfPU3AS4f

; CHECK: Test program was successfully built.
