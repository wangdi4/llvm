; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define i8 @f_xchg8(i8* %m, i8 %v, i8 %e) #0 {
; LPU_CHECK-LABEL: f_xchg8
; LPU_CHECK: atmcmpxchg8
entry:
  %0 = cmpxchg i8* %m, i8 %e, i8 %v seq_cst seq_cst
  %1 = extractvalue { i8, i1 } %0, 1
  %conv = zext i1 %1 to i8
  ret i8 %conv
}

