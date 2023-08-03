; This test is to check the correct builtin for subgroup collectives called.

; RUN: SATest -BUILD --config=%s.cfg -tsize=0 --dump-llvm-file - | FileCheck %s
; CHECK-NOT: @llvm.smin
; CHECK: @llvm.umin
