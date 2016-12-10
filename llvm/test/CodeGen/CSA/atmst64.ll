; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define i64 @f_atmst64(i64* %where, i64 %what) #0 {
; LPU_CHECK-LABEL: f_atmst64
; LPU_CHECK: st64
; LPU_CHECK-NOT: atmxchg
; LPU_CHECK-NOT: atmcmpxchg
; LPU_CHECK-NOT: atomic
; LPU_CHECK-NOT: sync
entry:
  store atomic i64 %what, i64* %where seq_cst, align 8
  ret i64 %what
}

