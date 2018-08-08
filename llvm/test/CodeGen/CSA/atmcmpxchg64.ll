; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i64 @f_xchg64(i64* %m, i64 %v, i64 %e) #0 {
; CSA_CHECK-LABEL: f_xchg64
; CSA_CHECK: .result .lic .i1 %[[OUTORD:[a-z0-9_]+]]
; CSA_CHECK: .result .lic .i64 %[[RES:[a-z0-9_]+]]
; CSA_CHECK: .param .lic .i1 %[[INORD:[a-z0-9_]+]]
; CSA_CHECK: .param .lic .i64 %[[ADDR:[a-z0-9_]+]]
; CSA_CHECK: .param .lic .i64 %[[REPL:[a-z0-9_]+]]
; CSA_CHECK: .param .lic .i64 %[[CMP:[a-z0-9_]+]]
; CSA_CHECK: atmcmpxchg64 %[[OUTORD:[a-z0-9_]+]], %[[RES:[a-z0-9_]+]], %[[ADDR]], %[[CMP]], %[[REPL]], %[[INORD]]
entry:
  %0 = cmpxchg i64* %m, i64 %e, i64 %v seq_cst seq_cst
  %1 = extractvalue { i64, i1 } %0, 0
  ret i64 %1
}

