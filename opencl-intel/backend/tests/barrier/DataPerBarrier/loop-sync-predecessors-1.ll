; RUN: llvm-as %s -o %t.bc
; RUN: opt -analyze -B-BarrierAnalysis -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the DataPerBarrier pass
;; The case: kernel "main" with barrier instruction in a loop (no fiber)
;; The expected result:
;;      0. Kernel "main" was not changed
;;  synchronize basic blocks barrier predecessors analysis data collected as follow
;;      1. Data collected only for basic block "L1"
;;      2. It has no fiber instruction as its predecessor
;;      3. The only sync basic block in his predecessors is "L1" itself
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

; CHECK: synchronize basic blocks successors

; CHECK: synchronize basic blocks barrier predecessors
; CHECK-NOT: +
; CHECK: +L1
; CHECK: has fiber instruction as predecessors: 0
; CHECK-NOT: -
; CHECK-NOT: +
; CHECK: -L1
; CHECK-NOT: -
; CHECK-NOT: +
; CHECK: *
; CHECK-NOT: +

; CHECK: DONE