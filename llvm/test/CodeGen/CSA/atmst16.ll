; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i16 @f_atmst16(i16* %where, i16 %what) #0 {
; CSA_CHECK-LABEL: f_atmst16
; CSA_CHECK: st16
; CSA_CHECK-NOT: atmxchg
; CSA_CHECK-NOT: atmcmpxchg
; CSA_CHECK-NOT: atomic
; CSA_CHECK-NOT: sync
entry:
  store atomic i16 %what, i16* %where seq_cst, align 8
  ret i16 %what
}

