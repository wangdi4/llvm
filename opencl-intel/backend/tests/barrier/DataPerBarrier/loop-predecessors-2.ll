; RUN: llvm-as %s -o %t.bc
; RUN: opt -analyze -B-BarrierAnalysis -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the DataPerBarrier pass
;; The case: kernel "main" with barrier instruction in loop
;; The expected result:
;;      0. Kernel "main" was not changed
;;  basic blocks predecessors analysis data collected as follow
;;      1. Data collected for basic block "L1"
;;      2. "L0" basic block is in "L1" predecessors
;;      3. "Exit" basic block is not in "L1" predecessors
;;*****************************************************************************

; CHECK: @main
define void @main(i32 %x) nounwind {
Entry:
  %check = icmp ult i32 %x, 0
  br label %L0
L0:
  %isOk = phi i1 [ false, %L1 ], [ true, %Entry ]
  br label %L1
L1:
  call void @barrier(i32 2)
  br i1 %check, label %L0, label %Exit
Exit:
  ret void
; CHECK: %check = icmp ult i32 %x, 0
; CHECK: br label %L0
; CHECK: L0:
; CHECK: %isOk = phi i1 [ false, %L1 ], [ true, %Entry ]
; CHECK: br label %L1
; CHECK: L1:
; CHECK: call void @barrier(i32 2)
; CHECK: br i1 %check, label %L0, label %Exit
; CHECK: Exit:
; CHECK: ret void
}

declare void @barrier(i32)

; CHECK: synchronize basic blocks

; CHECK: basic blocks predecessors
; CHECK: +L1
; CHECK-NOT: +
; CHECK-NOT: Exit
; CHECK: -L0
; CHECK-NOT: Exit
; CHECK: *

; CHECK: synchronize basic blocks successors

; CHECK: synchronize basic blocks barrier predecessors

; CHECK: DONE