; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define i16 @f_atm16(i16* %m, i16 signext %v) #0 {
; LPU_CHECK-LABEL: f_atm16
; LPU_CHECK: atm16
entry:
  %0 = atomicrmw add i16* %m, i16 %v seq_cst
  ret i16 %0
}

