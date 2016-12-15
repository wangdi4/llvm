; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i64 @f_atmld64(i64* %m) #0 {
; CSA_CHECK-LABEL: f_atmld64
; CSA_CHECK: ld64
; CSA_CHECK-NOT: atmxchg
; CSA_CHECK-NOT: atmcmpxchg
; CSA_CHECK-NOT: atomic
; CSA_CHECK-NOT: sync
entry:
  %0 = load atomic i64, i64* %m seq_cst, align 8
  ret i64 %0
}

