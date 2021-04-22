; RUN: opt -disable-output 2>&1 -passes='print<dpcpp-kernel-data-per-barrier-analysis>' %s | FileCheck %s
; RUN: opt -analyze -dpcpp-kernel-data-per-barrier-analysis %s -S -o - | FileCheck %s

;;*****************************************************************************
;; This test checks the DataPerBarrier pass
;; The case: kernel "main" with barrier instruction in loop
;; The expected result:
;;      0. Kernel "main" was not changed
;;  basic blocks predecessors analysis data collected as follow
;;      1. Data collected for basic block "L1"
;;      2. "Entry" basic block is in "L1" predecessors
;;      3. "Exit" basic block is not in "L1" predecessors
;;*****************************************************************************

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK-LABEL: @main
define void @main(i32 %x) {
Entry:
  %check = icmp ult i32 %x, 0
  br label %L0
L0:
  %isOk = phi i1 [ false, %L1 ], [ true, %Entry ]
  br label %L1
L1:
  call void @_Z18work_group_barrierj(i32 2)
  br i1 %check, label %L0, label %Exit
Exit:
  ret void
; CHECK: %check = icmp ult i32 %x, 0
; CHECK: br label %L0
; CHECK: L0:
; CHECK: %isOk = phi i1 [ false, %L1 ], [ true, %Entry ]
; CHECK: br label %L1
; CHECK: L1:
; CHECK: call void @_Z18work_group_barrierj(i32 2)
; CHECK: br i1 %check, label %L0, label %Exit
; CHECK: Exit:
; CHECK: ret void
}

declare void @_Z18work_group_barrierj(i32)

; CHECK: synchronize basic blocks

; CHECK: basic blocks predecessors
; CHECK: +L1
; CHECK-NOT: +
; CHECK-NOT: Exit
; CHECK: -Entry
; CHECK-NOT: Exit
; CHECK: *

; CHECK: synchronize basic blocks successors

; CHECK: synchronize basic blocks barrier predecessors

; CHECK: DONE
