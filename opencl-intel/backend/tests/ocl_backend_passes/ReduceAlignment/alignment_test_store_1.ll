; RUN: opt -reduce-alignment -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

;;*****************************************************************************
;; This test checks the reduce alignment pass
;; The case: alignment_test has store with align 1
;; The expected result:
;;      1. the store will stay with align 1
;;      2. since the align 1 is part of align 16 we'll check not 6 after
;;*****************************************************************************

declare noalias <4 x float>* @malloc(i64)

define void @alignment_test(<4 x float> %a, <4 x float>* nocapture %t) {
entry:
  %call = tail call <4 x float>* @malloc(i64 64)
  store <4 x float> %a, <4 x float>* %call, align 1
  ret void
}

; CHECK: %call = tail call <4 x float>* @malloc(i64 64)
; CHECK: store <4 x float> %a, <4 x float>* %call, align {{(1)$}}