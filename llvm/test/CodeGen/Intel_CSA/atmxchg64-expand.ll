; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i64 @f_atomic_xchg64(i64* %m, i64 signext %v) #0 {
; CSA_CHECK-LABEL: f_atomic_xchg64
; CSA_CHECK: atmcmpxchg64 %[[OUTORD:[0-9a-z_\.]+]], %[[GOT:[0-9a-z_\.]+]], %[[ADDR:[0-9a-z_\.]+]], %[[WANT:[0-9a-z_\.]+]], %[[OPNDS:[0-9a-z_\.]+]]
; CSA_CHECK: cmpne64
; CSA_CHECK-SAME:, %[[GOT]]
entry:
  %0 = atomicrmw xchg i64* %m, i64 %v seq_cst
  ret i64 %0
}

