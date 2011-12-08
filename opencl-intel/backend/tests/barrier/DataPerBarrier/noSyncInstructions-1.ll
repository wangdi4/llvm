; RUN: llvm-as %s -o %t.bc
; RUN: opt -analyze -B-BarrierAnalysis -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the DataPerBarrier pass
;; The case: kernel "main" with no barrier instruction
;; The expected result:
;;      0. Kernel "main" was not changed
;;      1. No analysis data was collected
;;*****************************************************************************

; CHECK: @main
define void @main(i32 %x) nounwind {
  %check = icmp ult i32 %x, 0
  br i1 %check, label %L1, label %L2
L1:
  br label %L3
L2:
  br label %L3
L3:
  %isOk = phi i1 [ false, %L1 ], [ true, %L2 ]
  ret void
; CHECK: %check = icmp ult i32 %x, 0
; CHECK: br i1 %check, label %L1, label %L2
; CHECK: L1:
; CHECK: br label %L3
; CHECK: L2:
; CHECK: br label %L3
; CHECK: L3:
; CHECK: %isOk = phi i1 [ false, %L1 ], [ true, %L2 ]
; CHECK: ret void
}

; CHECK: synchronize basic blocks
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *

; CHECK: basic blocks predecessors
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *

; CHECK: synchronize basic blocks successors
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *

; CHECK: synchronize basic blocks barrier predecessors
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *
; CHECK: DONE