; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i16 @f_atomic_xchg16(i16* %m, i16 signext %v) #0 {
; CSA_CHECK-LABEL: f_atomic_xchg16
; CSA_CHECK: atmcmpxchg16 %[[OUTORD:[0-9a-z_\.]+]], %[[GOT:[0-9a-z_\.]+]], %[[ADDR:[0-9a-z_\.]+]], %[[WANT:[0-9a-z_\.]+]], %[[OPNDS:[0-9a-z_\.]+]]
; CSA_CHECK: cmpne16
; CSA_CHECK-SAME:, %[[GOT]]
entry:
  %0 = atomicrmw xchg i16* %m, i16 %v seq_cst
  ret i16 %0
}

