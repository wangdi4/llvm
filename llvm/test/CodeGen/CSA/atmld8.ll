; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i8 @f_atmld8(i8* %m) #0 {
; CSA_CHECK-LABEL: f_atmld8
; CSA_CHECK: ld8
; CSA_CHECK-NOT: atmxchg
; CSA_CHECK-NOT: atmcmpxchg
; CSA_CHECK-NOT: atomic
; CSA_CHECK-NOT: sync
entry:
  %0 = load atomic i8, i8* %m seq_cst, align 8
  ret i8 %0
}

