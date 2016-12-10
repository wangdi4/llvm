; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define i16 @f_atmmin16(i16* %m, i16 signext %v) #0 {
; LPU_CHECK-LABEL: f_atmmin16
; LPU_CHECK: atmmin16
entry:
  %0 = atomicrmw min i16* %m, i16 %v seq_cst
  ret i16 %0
}

