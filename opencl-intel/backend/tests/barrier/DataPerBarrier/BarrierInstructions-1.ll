; RUN: llvm-as %s -o %t.bc
; RUN: opt -analyze -B-BarrierAnalysis -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the DataPerBarrier pass
;; The case: kernel "main" with barrier instruction
;; The expected result:
;;      0. Kernel "main" was not changed
;;  synchronize basic blocks analysis data collected as follow
;;      1. Data collected only for function "main"
;;      2. Data collected only for basic block "L0"
;;  synchronize basic blocks successors analysis data collected as follow
;;      3. Data collected only for basic block "L0"
;;  synchronize basic blocks barrier predecessors analysis data collected as follow
;;      4. Data collected only for basic block "L0"
;;*****************************************************************************

; CHECK: @main
define void @main(i32 %x) nounwind {
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
; CHECK-NOT: +
; CHECK: +main
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK: -L0
; CHECK-NOT: -
; CHECK-NOT: +
; CHECK: *

; CHECK: basic blocks predecessors

; CHECK: synchronize basic blocks successors
; CHECK-NOT: +
; CHECK: +L0
; CHECK-NOT: +
; CHECK: *

; CHECK: synchronize basic blocks barrier predecessors
; CHECK-NOT: +
; CHECK: +L0
; CHECK-NOT: +
; CHECK: *
; CHECK: DONE