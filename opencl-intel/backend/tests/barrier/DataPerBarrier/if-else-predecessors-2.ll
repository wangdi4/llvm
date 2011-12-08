; RUN: llvm-as %s -o %t.bc
; RUN: opt -analyze -B-BarrierAnalysis -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the DataPerBarrier pass
;; The case: kernel "main" with barrier instruction in pre if-else basic block
;; The expected result:
;;      0. Kernel "main" was not changed
;;  basic blocks predecessors analysis data collected as follow
;;      1. Data collected for basic block "L3"
;;      2. "L0" basic block is in "L3" predecessors
;;      3. "L3" itself is not in "L3" predecessors
;;*****************************************************************************

; CHECK: @main
define void @main(i32 %x) nounwind {
Entry:
  %check = icmp ult i32 %x, 0
  br label %L0
L0:
  call void @barrier(i32 2)
  br i1 %check, label %L1, label %L2
L1:
  br label %L3
L2:
  br label %L3
L3:
  %isOk = phi i1 [ false, %L1 ], [ true, %L2 ]
  ret void
; CHECK: %check = icmp ult i32 %x, 0
; CHECK: br label %L0
; CHECK: L0:
; CHECK: call void @barrier(i32 2)
; CHECK: br i1 %check, label %L1, label %L2
; CHECK: L1:
; CHECK: br label %L3
; CHECK: L2:
; CHECK: br label %L3
; CHECK: L3:
; CHECK: %isOk = phi i1 [ false, %L1 ], [ true, %L2 ]
; CHECK: ret void
}

declare void @barrier(i32)

; CHECK: synchronize basic blocks

; CHECK: basic blocks predecessors
; CHECK: +L3
; CHECK-NOT: +
; CHECK-NOT: L3
; CHECK: -L0
; CHECK-NOT: L3
; CHECK: *

; CHECK: synchronize basic blocks successors

; CHECK: synchronize basic blocks barrier predecessors

; CHECK: DONE