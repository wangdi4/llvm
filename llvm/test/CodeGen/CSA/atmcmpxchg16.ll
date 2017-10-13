; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i16 @f_xchg16(i16* %m, i16 %v, i16 %e) #0 {
; CSA_CHECK-LABEL: f_xchg16
; CSA_CHECK: atmcmpxchg16
entry:
  %0 = cmpxchg i16* %m, i16 %e, i16 %v seq_cst seq_cst
  %1 = extractvalue { i16, i1 } %0, 1
  %conv = zext i1 %1 to i16
  ret i16 %conv
}

