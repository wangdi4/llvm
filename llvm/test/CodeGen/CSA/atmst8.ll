; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i8 @f_atmst8(i8* %where, i8 %what) #0 {
; CSA_CHECK-LABEL: f_atmst8
; CSA_CHECK: st8
; CSA_CHECK-NOT: atmxchg
; CSA_CHECK-NOT: atmcmpxchg
; CSA_CHECK-NOT: atomic
; CSA_CHECK-NOT: sync
entry:
  store atomic i8 %what, i8* %where seq_cst, align 8
  ret i8 %what
}

