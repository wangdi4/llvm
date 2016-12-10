; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define i32 @f_atmst32(i32* %where, i32 %what) #0 {
; LPU_CHECK-LABEL: f_atmst32
; LPU_CHECK: st32
; LPU_CHECK-NOT: atmxchg
; LPU_CHECK-NOT: atmcmpxchg
; LPU_CHECK-NOT: atomic
; LPU_CHECK-NOT: sync
entry:
  store atomic i32 %what, i32* %where seq_cst, align 8
  ret i32 %what
}

