; RUN: opt -passes=sycl-kernel-split-on-barrier -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-split-on-barrier  -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the SplitBBonBarrier pass
;; The case: function "main" with 1 dummybarrier instruction in middle of its basic block
;; The expected result:
;;      1. 1 dummybarrier instruction start at begining of basic block
;;      2. All the instruction stay in the same orders
;;*****************************************************************************
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

; CHECK: @main
define void @main(i32 %x) nounwind {
  %check = icmp ult i32 %x, 0
  call void @dummy_barrier.()
  br i1 %check, label %L1, label %L2
L1:
  br label %L3
L2:
  br label %L3
L3:
  %isOk = phi i1 [ false, %L1 ], [ true, %L2 ]
  ret void
; CHECK: %check = icmp ult i32 %x, 0
; CHECK: :
; CHECK: call void @dummy_barrier.()
; CHECK: br i1 %check, label %L1, label %L2
; CHECK: L1:
; CHECK: br label %L3
; CHECK: L2:
; CHECK: br label %L3
; CHECK: L3:
; CHECK: %isOk = phi i1 [ false, %L1 ], [ true, %L2 ]
; CHECK: ret void
}

declare void @dummy_barrier.()

; DEBUGIFY-NOT: WARNING
