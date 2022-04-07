; This test is to check the correct builtin for subgroup collectives called.

; llvm.umax intrinics is called with transpose size=16.
; RUN: SATest -BUILD --config=%s.cfg -tsize=16 --cpuarch=skx --dump-llvm-file - | FileCheck %s
; CHECK-NOT: @llvm.smax
; CHECK: @llvm.umax
