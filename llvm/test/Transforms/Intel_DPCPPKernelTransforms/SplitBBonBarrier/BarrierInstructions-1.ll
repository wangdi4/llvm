; RUN: opt -dpcpp-split-on-barrier %s -S -o - | FileCheck %s

;;*****************************************************************************
;; This test checks the SplitBBonBarrier pass
;; The case: function "main" with 1 barrier instruction in middle of its basic block
;; The expected result:
;;      1. 1 barrier instruction start at begining of basic block
;;      2. All the instruction stay in the same orders
;;*****************************************************************************
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

; CHECK: @main
define void @main(i32 %x) nounwind {
  %check = icmp ult i32 %x, 0
  call void @__builtin_dpcpp_kernel_barrier(i32 2)
  br i1 %check, label %L1, label %L2
L1:
  br label %L3
L2:
  br label %L3
L3:
  %isOk = phi i1 [ false, %L1 ], [ true, %L2 ]
  ret void
; CHECK-NEXT: %check = icmp ult i32 %x, 0
; CHECK-NEXT: br label %[[BARRIER_BB:.*]]
; CHECK:      [[BARRIER_BB]]:
; CHECK-NEXT: call void @__builtin_dpcpp_kernel_barrier(i32 2)
; CHECK-NEXT: br i1 %check, label %L1, label %L2
; CHECK:      L1:
; CHECK-NEXT: br label %L3
; CHECK:      L2:
; CHECK-NEXT: br label %L3
; CHECK:      L3:
; CHECK-NEXT: %isOk = phi i1 [ false, %L1 ], [ true, %L2 ]
; CHECK-NEXT: ret void
}

declare void @__builtin_dpcpp_kernel_barrier(i32)
