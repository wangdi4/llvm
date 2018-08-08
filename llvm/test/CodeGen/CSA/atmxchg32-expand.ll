; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i32 @f_atomic_xchg32(i32* %m, i32 signext %v) #0 {
; CSA_CHECK-LABEL: f_atomic_xchg32
; CSA_CHECK: atmcmpxchg32 %[[OUTORD:[0-9a-z_\.]+]], %[[GOT:[0-9a-z_\.]+]], %[[ADDR:[0-9a-z_\.]+]], %[[WANT:[0-9a-z_\.]+]], %[[OPNDS:[0-9a-z_\.]+]]
; CSA_CHECK: cmpne32
; CSA_CHECK-SAME:, %[[GOT]]
entry:
  %0 = atomicrmw xchg i32* %m, i32 %v seq_cst
  ret i32 %0
}

