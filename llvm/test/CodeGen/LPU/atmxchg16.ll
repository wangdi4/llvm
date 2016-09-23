; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define i16 @f_xchg16(i16* %m, i16 %v) #0 {
; LPU_CHECK-LABEL: f_xchg16
; LPU_CHECK: atmxchg16
entry:
  %0 = atomicrmw xchg i16* %m, i16 %v seq_cst
    ret i16 %0
}

