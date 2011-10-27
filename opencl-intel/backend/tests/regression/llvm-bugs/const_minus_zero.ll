; Reproducer test for CSSD100005558

; RUN: llc < %s | FileCheck %s
; CHECK: .quad   -9223372036854775808

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define double  @kernel8(double %a, double %b, double %c) nounwind {
  %a11 = fmul double %a, %b
  %a12 = fsub double %c, %a11
  %a13 = fsub double -0.000000e+000, %a12
  ret double %a13
}
