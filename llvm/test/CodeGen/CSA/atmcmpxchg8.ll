; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i8 @f_xchg8(i8* %m, i8 %v, i8 %e) #0 {
; CSA_CHECK-LABEL: f_xchg8
; CSA_CHECK-DAG: mov0 %[[INORD:[a-z0-9_]+]], %ra
; CSA_CHECK-DAG: mov{{8|64}} %[[ADDR:[a-z0-9_]+]], %r2
; CSA_CHECK-DAG: mov{{8|64}} %[[REPL:[a-z0-9_]+]], %r3
; CSA_CHECK-DAG: mov{{8|64}} %[[CMP:[a-z0-9_]+]],  %r4
; CSA_CHECK-DAG: atmcmpxchg8 %[[OUTORD:[a-z0-9_]+]], %[[RES:[a-z0-9_]+]], %[[ADDR]], %[[CMP]], %[[REPL]], %[[INORD]]
; CSA_CHECK-DAG: mov0 %r{{[0-9]+}}, %[[OUTORD]]
; CSA_CHECK-DAG: mov{{8|64}} %r0, %[[RES]]
entry:
  %0 = cmpxchg i8* %m, i8 %e, i8 %v seq_cst seq_cst
  %1 = extractvalue { i8, i1 } %0, 0
  ret i8 %1
}

