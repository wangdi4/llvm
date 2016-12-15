; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i32 @f_atmst32(i32* %where, i32 %what) #0 {
; CSA_CHECK-LABEL: f_atmst32
; CSA_CHECK: st32
; CSA_CHECK-NOT: atmxchg
; CSA_CHECK-NOT: atmcmpxchg
; CSA_CHECK-NOT: atomic
; CSA_CHECK-NOT: sync
entry:
  store atomic i32 %what, i32* %where seq_cst, align 8
  ret i32 %what
}

