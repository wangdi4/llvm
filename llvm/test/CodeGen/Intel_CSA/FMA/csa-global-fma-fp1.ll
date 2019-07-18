; RUN: llc <%s -mtriple=csa -fp-contract=fast -enable-unsafe-fp-math -csa-global-fma | FileCheck %s

; This test checks that Global FMA optimizes the following test case
; using 1.0 fp const.
; For the input expression:
;   +acd+a+b
; the output code must have 2 arithmetic instructions:
;   F0=+c*d+1; F1=+F0*a+b;

define float @test_ss(float %a, float %b, float %c, float %d) {
; CHECK:       .entry  test_ss
; CHECK-NEXT:  .result .lic .i0
; CHECK-NEXT:  .result .lic .i32 [[RET:%.+]]
; CHECK-NEXT:  .param .lic .i0
; CHECK-NEXT:  .param .lic .i32 [[A:%.+]]
; CHECK-NEXT:  .param .lic .i32 [[B:%.+]]
; CHECK-NEXT:  .param .lic .i32 [[C:%.+]]
; CHECK-NEXT:  .param .lic .i32 [[D:%.+]]
; CHECK:       fmaf32  [[T0:%.+]], [[D]], [[C]], 0x3f800000
; CHECK-NEXT:  fmaf32  [[RET]], [[T0]], [[A]], [[B]]
entry:
  %mul = fmul fast float %c, %a
  %mul1 = fmul fast float %mul, %d
  %add = fadd fast float %b, %a
  %add2 = fadd fast float %add, %mul1
  ret float %add2
}

define double @test_sd(double %a, double %b, double %c, double %d) {
; CHECK:       .entry  test_sd
; CHECK-NEXT:  .result .lic .i0
; CHECK-NEXT:  .result .lic .i64 [[RET:%.+]]
; CHECK-NEXT:  .param .lic .i0
; CHECK-NEXT:  .param .lic .i64 [[A:%.+]]
; CHECK-NEXT:  .param .lic .i64 [[B:%.+]]
; CHECK-NEXT:  .param .lic .i64 [[C:%.+]]
; CHECK-NEXT:  .param .lic .i64 [[D:%.+]]
; CHECK:       fmaf64  [[T0:%.+]], [[D]], [[C]], 0x3ff0000000000000
; CHECK-NEXT:  fmaf64  [[RET]], [[T0]], [[A]], [[B]]
entry:
  %mul = fmul fast double %c, %a
  %mul1 = fmul fast double %mul, %d
  %add = fadd fast double %b, %a
  %add2 = fadd fast double %add, %mul1
  ret double %add2
}

define <2 x float> @test_f32x2(<2 x float> %a, <2 x float> %b, <2 x float> %c, <2 x float> %d) {
; CHECK:       .entry  test_f32x2
; CHECK-NEXT:  .result .lic .i0
; CHECK-NEXT:  .result .lic .i64 [[RET:%.+]]
; CHECK-NEXT:  .param .lic .i0
; CHECK-NEXT:  .param .lic .i64 [[A:%.+]]
; CHECK-NEXT:  .param .lic .i64 [[B:%.+]]
; CHECK-NEXT:  .param .lic .i64 [[C:%.+]]
; CHECK-NEXT:  .param .lic .i64 [[D:%.+]]
; CHECK:       fmaf32x2        [[T0:%.+]], [[D]], [[C]], 0x3f8000003f800000, 0, 0, 0
; CHECK-NEXT:  fmaf32x2        [[RET]], [[T0]], [[A]], [[B]], 0, 0, 0
entry:
  %mul = fmul fast <2 x float> %c, %a
  %mul1 = fmul fast <2 x float> %mul, %d
  %add = fadd fast <2 x float> %b, %a
  %add2 = fadd fast <2 x float> %add, %mul1
  ret <2 x float> %add2
}
