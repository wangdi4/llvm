; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define i8 @f_atm8(i8* %m, i8 signext %v) #0 {
; LPU_CHECK-LABEL: f_atm8
; LPU_CHECK: atm8
entry:
  %0 = atomicrmw add i8* %m, i8 %v seq_cst
  ret i8 %0
}

