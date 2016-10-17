; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define i16 @f_atmld16(i16* %m) #0 {
; LPU_CHECK-LABEL: f_atmld16
; LPU_CHECK: ld16
; LPU_CHECK-NOT: atmxchg
; LPU_CHECK-NOT: atmcmpxchg
; LPU_CHECK-NOT: atomic
; LPU_CHECK-NOT: sync
entry:
  %0 = load atomic i16, i16* %m seq_cst, align 8
  ret i16 %0
}

