; RUN: opt -instcombine -S -o - %s | FileCheck %s

; This test verifies that calls are not constant folded when there are
; imf attributes attached to the call site.

; The calls in this function have imf attributes and should not get folded.
define void @f1() {
  %t1 = call double @llvm.sin.f64(double 0x3FE921FB54442D18) #1
  call void @g(double %t1)
  %t2 = call double @sin(double 0x3FE921FB54442D18) #1
  call void @g(double %t2)
  ret void
}
; CHECK-LABEL: @f1
; CHECK: call double @llvm.sin.f64(double 0x3FE921FB54442D18)
; CHECK: call double @sin(double 0x3FE921FB54442D18)

; The calls in this function have no imf attributes and should get folded.
define void @f2() {
  %t1 = call double @llvm.sin.f64(double 0x3FE921FB54442D18) #2
  call void @g(double %t1)
  %t2 = call double @sin(double 0x3FE921FB54442D18) #2
  call void @g(double %t2)
  ret void
}
; CHECK-LABEL: @f2
; Note: The last digit of the folded constant is omitted in the checks below
;       to avoid potential instability in the test.
; CHECK-NOT: call double @llvm.sin.f64(double 0x3FE921FB54442D18)
; CHECK: call void @g(double 0x3FE6A09E667F3BC
; CHECK-NOT: call double @sin(double 0x3FF921FB54442D18)
; CHECK: call void @g(double 0x3FE6A09E667F3BC

declare double @llvm.sin.f64(double)
declare double @sin(double)
declare void @g(double)

attributes #1 = { nounwind "imf-arch-consistency"="true" }
attributes #2 = { nounwind }