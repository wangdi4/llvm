; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i32 @f_xchg32(i32* %m, i32 %v, i32 %e) #0 {
; CSA_CHECK-LABEL: f_xchg32
; CSA_CHECK: .result .lic .i1 %[[OUTORD:[a-z0-9_]+]]
; CSA_CHECK: .result .lic .i32 %[[RES:[a-z0-9_]+]]
; CSA_CHECK: .param .lic .i1 %[[INORD:[a-z0-9_]+]]
; CSA_CHECK: .param .lic .i64 %[[ADDR:[a-z0-9_]+]]
; CSA_CHECK: .param .lic .i32 %[[REPL:[a-z0-9_]+]]
; CSA_CHECK: .param .lic .i32 %[[CMP:[a-z0-9_]+]]
; CSA_CHECK: atmcmpxchg32 %[[OUTORD:[a-z0-9_]+]], %[[RES:[a-z0-9_]+]], %[[ADDR]], %[[CMP]], %[[REPL]], %[[INORD]]
entry:
  %0 = cmpxchg i32* %m, i32 %e, i32 %v seq_cst seq_cst
  %1 = extractvalue { i32, i1 } %0, 0
  ret i32 %1
}

