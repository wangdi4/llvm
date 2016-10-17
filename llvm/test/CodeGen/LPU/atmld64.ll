; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define i64 @f_atmld64(i64* %m) #0 {
; LPU_CHECK-LABEL: f_atmld64
; LPU_CHECK: ld64
; LPU_CHECK-NOT: atmxchg
; LPU_CHECK-NOT: atmcmpxchg
; LPU_CHECK-NOT: atomic
; LPU_CHECK-NOT: sync
entry:
  %0 = load atomic i64, i64* %m seq_cst, align 8
  ret i64 %0
}

