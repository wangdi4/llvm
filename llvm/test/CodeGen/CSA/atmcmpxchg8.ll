; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i8 @f_xchg8(i8* %m, i8 %v, i8 %e) #0 {
; CSA_CHECK-LABEL: f_xchg8
; CSA_CHECK: .result .lic .i1 %[[OUTORD:[a-z0-9_]+]]
; CSA_CHECK: .result .lic .i8 %[[RES:[a-z0-9_]+]]
; CSA_CHECK: .param .lic .i1 %[[INORD:[a-z0-9_]+]]
; CSA_CHECK: .param .lic .i64 %[[ADDR:[a-z0-9_]+]]
; CSA_CHECK: .param .lic .i8 %[[REPL:[a-z0-9_]+]]
; CSA_CHECK: .param .lic .i8 %[[CMP:[a-z0-9_]+]]
; CSA_CHECK: atmcmpxchg8 %[[OUTORD:[a-z0-9_]+]], %[[RES:[a-z0-9_]+]], %[[ADDR]], %[[CMP]], %[[REPL]], %[[INORD]]
entry:
  %0 = cmpxchg i8* %m, i8 %e, i8 %v seq_cst seq_cst
  %1 = extractvalue { i8, i1 } %0, 0
  ret i8 %1
}

