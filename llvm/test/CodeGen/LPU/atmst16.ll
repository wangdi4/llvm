; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define i16 @f_atmst16(i16* %where, i16 %what) #0 {
; LPU_CHECK-LABEL: f_atmst16
; LPU_CHECK: st16
; LPU_CHECK-NOT: atmxchg
; LPU_CHECK-NOT: atmcmpxchg
; LPU_CHECK-NOT: atomic
; LPU_CHECK-NOT: sync
entry:
  store atomic i16 %what, i16* %where seq_cst, align 8
  ret i16 %what
}

