; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define i8 @f_atmxchg8(i8* %m, i8 signext %v) #0 {
; LPU_CHECK-LABEL: f_atmxchg8
; LPU_CHECK: atmxchg8
entry:
  %0 = atomicrmw xchg i8* %m, i8 %v seq_cst
  ret i8 %0
}

