; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define i64 @f_atm64(i64* %m, i64 signext %v) #0 {
; LPU_CHECK-LABEL: f_atm64
; LPU_CHECK: atm64
entry:
  %0 = atomicrmw add i64* %m, i64 %v seq_cst
  ret i64 %0
}

