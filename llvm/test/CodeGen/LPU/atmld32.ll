; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define i32 @f_atmld32(i32* %m) #0 {
; LPU_CHECK-LABEL: f_atmld32
; LPU_CHECK: ld32
; LPU_CHECK-NOT: atmxchg
; LPU_CHECK-NOT: atmcmpxchg
; LPU_CHECK-NOT: atomic
; LPU_CHECK-NOT: sync
entry:
  %0 = load atomic i32, i32* %m seq_cst, align 8
  ret i32 %0
}

