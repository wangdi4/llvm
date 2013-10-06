; RUN: opt -reduce-alignment -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

;;*****************************************************************************
;; This test checks the reduce alignment pass
;; The case: alignment_test has load with align 1
;; The expected result:
;;      1. the load will stay with align 1
;;      2. since the align 1 is part of align 16 we'll check not 6 after.
;;*****************************************************************************

define void @alignment_test(<4 x float>* nocapture %x) {
entry:
  %0 = bitcast <4 x float>* %x to <4 x i32>*
  %1 = load <4 x i32>* %0, align 1
  ret void
}

; CHECK: %0 = bitcast <4 x float>* %x to <4 x i32>*
; CHECK: %1 = load <4 x i32>* %0, align {{(1)$}}