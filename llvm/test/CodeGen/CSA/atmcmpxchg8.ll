; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i8 @f_xchg8(i8* %m, i8 %v, i8 %e) #0 {
; CSA_CHECK-LABEL: f_xchg8
; CSA_CHECK: atmcmpxchg8
entry:
  %0 = cmpxchg i8* %m, i8 %e, i8 %v seq_cst seq_cst
  %1 = extractvalue { i8, i1 } %0, 1
  %conv = zext i1 %1 to i8
  ret i8 %conv
}

