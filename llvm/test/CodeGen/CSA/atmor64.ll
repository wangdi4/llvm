; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define i64 @f_atmor64(i64* %m, i64 signext %v) #0 {
; LPU_CHECK-LABEL: f_atmor64
; LPU_CHECK: atmor64
entry:
  %0 = atomicrmw or i64* %m, i64 %v seq_cst
  ret i64 %0
}

