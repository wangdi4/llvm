; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i64 @f_atmadd64(i64* %m, i64 signext %v) #0 {
; CSA_CHECK-LABEL: f_atmadd64
; CSA_CHECK: atmadd64
entry:
  %0 = atomicrmw add i64* %m, i64 %v seq_cst
  ret i64 %0
}

