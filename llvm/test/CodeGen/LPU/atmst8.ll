; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define i8 @f_atmst8(i8* %where, i8 %what) #0 {
; LPU_CHECK-LABEL: f_atmst8
; LPU_CHECK: st8
; LPU_CHECK-NOT: atmxchg
; LPU_CHECK-NOT: atmcmpxchg
; LPU_CHECK-NOT: atomic
; LPU_CHECK-NOT: sync
entry:
  store atomic i8 %what, i8* %where seq_cst, align 8
  ret i8 %what
}

