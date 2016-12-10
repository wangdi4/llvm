; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define i8 @f_atmld8(i8* %m) #0 {
; LPU_CHECK-LABEL: f_atmld8
; LPU_CHECK: ld8
; LPU_CHECK-NOT: atmxchg
; LPU_CHECK-NOT: atmcmpxchg
; LPU_CHECK-NOT: atomic
; LPU_CHECK-NOT: sync
entry:
  %0 = load atomic i8, i8* %m seq_cst, align 8
  ret i8 %0
}

