; RUN: llc <%s -mtriple=csa -fp-contract=fast -enable-unsafe-fp-math -csa-global-fma | FileCheck %s

; This test checks that Global FMA optimizes test wit FP immediate.

define float @test_ss(float %a, float %b, float %c) {
; CHECK:       .entry  test_ss
; CHECK-NEXT:  .result .lic .i0
; CHECK-NEXT:  .result .lic .i32 [[RET:%.+]]
; CHECK-NEXT:  .param .lic .i0
; CHECK-NEXT:  .param .lic .i32 [[A:%.+]]
; CHECK-NEXT:  .param .lic .i32 [[B:%.+]]
; CHECK-NEXT:  .param .lic .i32 [[C:%.+]]
; CHECK:       fmaf32  [[T0:%.+]], [[A]], [[B]], 0x3f800000
; CHECK-NEXT:  fmaf32  [[RET]], [[T0]], [[C]], 0x3f000000
entry:
  %mul = fmul fast float %a, %b
  %mul1 = fmul fast float %mul, %c
  %add = fadd fast float %c, 0.5
  %add2 = fadd fast float %add, %mul1
  ret float %add2
}

define double @test_sd(double %a, double %b, double %c) {
; CHECK:       .entry  test_sd
; CHECK-NEXT:  .result .lic .i0
; CHECK-NEXT:  .result .lic .i64 [[RET:%.+]]
; CHECK-NEXT:  .param .lic .i0
; CHECK-NEXT:  .param .lic .i64 [[A:%.+]]
; CHECK-NEXT:  .param .lic .i64 [[B:%.+]]
; CHECK-NEXT:  .param .lic .i64 [[C:%.+]]
; CHECK:       fmaf64  [[T0:%.+]], [[A]], [[B]], 0x3ff0000000000000
; CHECK-NEXT:  fmaf64  [[RET]], [[T0]], [[C]], 0x3fe0000000000000
entry:
  %mul = fmul fast double %a, %b
  %mul1 = fmul fast double %mul, %c
  %add = fadd fast double %c, 0.5
  %add2 = fadd fast double %add, %mul1
  ret double %add2
}
