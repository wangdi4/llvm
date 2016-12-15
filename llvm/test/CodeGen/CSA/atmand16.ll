; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i16 @f_atmand16(i16* %m, i16 signext %v) #0 {
; CSA_CHECK-LABEL: f_atmand16
; CSA_CHECK: atmand16
entry:
  %0 = atomicrmw and i16* %m, i16 %v seq_cst
  ret i16 %0
}

