; RUN: opt -disable-output 2>&1 -passes='print<sycl-kernel-data-per-barrier-analysis>' %s | FileCheck %s

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
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

; CHECK-LABEL: @main
define void @main(i32 %x) {
  %check = icmp ult i32 %x, 0
  br label %L0
L0:
  call void @_Z18work_group_barrierj(i32 2)
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
; CHECK: call void @_Z18work_group_barrierj(i32 2)
; CHECK: br i1 %check, label %L1, label %L2
; CHECK: L1:
; CHECK: br label %L3
; CHECK: L2:
; CHECK: br label %L3
; CHECK: L3:
; CHECK: %isOk = phi i1 [ false, %L1 ], [ true, %L2 ]
; CHECK: ret void
}

declare void @_Z18work_group_barrierj(i32)

; CHECK: synchronize basic blocks
; CHECK-NOT: +
; CHECK: +main
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK: -L0
; CHECK-NOT: -
; CHECK-NOT: +
; CHECK: *

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
