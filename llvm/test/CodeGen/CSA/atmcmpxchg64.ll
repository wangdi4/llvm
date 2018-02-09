; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i64 @f_xchg64(i64* %m, i64 %v, i64 %e) #0 {
; CSA_CHECK-LABEL: f_xchg64
; CSA_CHECK-DAG: mov0 %[[INORD:[a-z0-9_]+]], %ra
; CSA_CHECK-DAG: mov64 %[[ADDR:[a-z0-9_]+]], %r2
; CSA_CHECK-DAG: mov64 %[[REPL:[a-z0-9_]+]], %r3
; CSA_CHECK-DAG: mov64 %[[CMP:[a-z0-9_]+]],  %r4
; CSA_CHECK-DAG: atmcmpxchg64 %[[OUTORD:[a-z0-9_]+]], %[[RES:[a-z0-9_]+]], %[[ADDR]], %[[CMP]], %[[REPL]], %[[INORD]]
; CSA_CHECK-DAG: mov0 %r{{[0-9]+}}, %[[OUTORD]]
; CSA_CHECK-DAG: mov64 %r0, %[[RES]]
entry:
  %0 = cmpxchg i64* %m, i64 %e, i64 %v seq_cst seq_cst
  %1 = extractvalue { i64, i1 } %0, 0
  ret i64 %1
}

