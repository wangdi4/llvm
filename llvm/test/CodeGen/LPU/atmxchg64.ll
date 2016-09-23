; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define i64 @f_xchg64(i64* %m, i64 %v) #0 {
; LPU_CHECK-LABEL: f_xchg64
; LPU_CHECK: atmxchg64
entry:
  %0 = atomicrmw xchg i64* %m, i64 %v seq_cst
    ret i64 %0
}

