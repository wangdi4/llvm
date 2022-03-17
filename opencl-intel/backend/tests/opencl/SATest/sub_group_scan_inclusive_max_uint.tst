; This test is to check the correct builtin for subgroup collectives called.

; RUN: SATest -BUILD -native-subgroups --config=%s.cfg -tsize=0 --dump-llvm-file - | FileCheck %s
; CHECK-NOT: @llvm.smax
; CHECK: @llvm.umax
