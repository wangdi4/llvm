; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i32 @f_xchg32(i32* %m, i32 %v, i32 %e) #0 {
; CSA_CHECK-LABEL: f_xchg32
; CSA_CHECK: atmcmpxchg32
entry:
  %0 = cmpxchg i32* %m, i32 %e, i32 %v seq_cst seq_cst
  %1 = extractvalue { i32, i1 } %0, 1
  %conv = zext i1 %1 to i32
  ret i32 %conv
}

