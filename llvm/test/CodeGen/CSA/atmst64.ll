; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i64 @f_atmst64(i64* %where, i64 %what) #0 {
; CSA_CHECK-LABEL: f_atmst64
; CSA_CHECK: st64
; CSA_CHECK-NOT: atmxchg
; CSA_CHECK-NOT: atmcmpxchg
; CSA_CHECK-NOT: atomic
; CSA_CHECK-NOT: sync
entry:
  store atomic i64 %what, i64* %where seq_cst, align 8
  ret i64 %what
}

