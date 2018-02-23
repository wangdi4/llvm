; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i32 @f_atomic_32(i32* %m, i32 signext %v) #0 {
; CSA_CHECK-LABEL: f_atomic_32
; CSA_CHECK: atmcmpxchg32
; CSA_CHECK: cmpne32
entry:
  %0 = atomicrmw add i32* %m, i32 %v seq_cst
  ret i32 %0
}

