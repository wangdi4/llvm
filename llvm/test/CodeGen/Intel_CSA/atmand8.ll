; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i8 @f_atomic_and8(i8* %m, i8 signext %v) #0 {
; CSA_CHECK-LABEL: f_atomic_and8
; CSA_CHECK: atmcmpxchg8
; CSA_CHECK: cmpne8
entry:
  %0 = atomicrmw and i8* %m, i8 %v seq_cst
  ret i8 %0
}

