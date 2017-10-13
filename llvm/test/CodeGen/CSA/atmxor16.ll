; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i16 @f_atm16(i16* %m, i16 signext %v) #0 {
; CSA_CHECK-LABEL: f_atm16
; CSA_CHECK: atm16
entry:
  %0 = atomicrmw add i16* %m, i16 %v seq_cst
  ret i16 %0
}

