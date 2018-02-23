; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i16 @f_xchg16(i16* %m, i16 %v, i16 %e) #0 {
; CSA_CHECK-LABEL: f_xchg16
; CSA_CHECK-DAG: mov0 %[[INORD:[a-z0-9_]+]], %ra
; CSA_CHECK-DAG: mov{{16|64}} %[[ADDR:[a-z0-9_]+]], %r2
; CSA_CHECK-DAG: mov{{16|64}} %[[REPL:[a-z0-9_]+]], %r3
; CSA_CHECK-DAG: mov{{16|64}} %[[CMP:[a-z0-9_]+]],  %r4
; CSA_CHECK-DAG: atmcmpxchg16 %[[OUTORD:[a-z0-9_]+]], %[[RES:[a-z0-9_]+]], %[[ADDR]], %[[CMP]], %[[REPL]], %[[INORD]]
; CSA_CHECK-DAG: mov0 %r{{[0-9]+}}, %[[OUTORD]]
; CSA_CHECK-DAG: mov{{16|64}} %r0, %[[RES]]
entry:
  %0 = cmpxchg i16* %m, i16 %e, i16 %v seq_cst seq_cst
  %1 = extractvalue { i16, i1 } %0, 0
  ret i16 %1
}

