; RUN: opt -passes=sycl-kernel-split-on-barrier -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-split-on-barrier -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the SplitBBonBarrier pass
;; The case: function "main" with 4 barrier instructions
;;           2 start at begining of its basic block while other 2 are in middle
;; The expected result:
;;      1. 4 barrier instructions each start at begining of basic block
;;      2. All the instruction stay in the same orders
;;*****************************************************************************
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

; CHECK: @main
define void @main(i32 %x) nounwind {
  %check = icmp ult i32 %x, 0
  call void @_Z18work_group_barrierj(i32 2)
  br i1 %check, label %L1, label %L2
L1:
  call void @_Z18work_group_barrierj(i32 1)
  call void @_Z18work_group_barrierj(i32 2)
  br label %L3
L2:
  call void @_Z18work_group_barrierj(i32 2)
  br label %L3
L3:
  %isOk = phi i1 [ false, %L1 ], [ true, %L2 ]
  call void @_Z18work_group_barrierj(i32 1)
  ret void
; CHECK: %check = icmp ult i32 %x, 0
; CHECK: br
; CHECK: :
; CHECK: call void @_Z18work_group_barrierj(i32 2)
; CHECK: br i1 %check, label %L1, label %L2
; CHECK: L1:
; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK: br
; CHECK: :
; CHECK: call void @_Z18work_group_barrierj(i32 2)
; CHECK: br label %L3
; CHECK: L2:
; CHECK: call void @_Z18work_group_barrierj(i32 2)
; CHECK: br label %L3
; CHECK: L3:
; CHECK: %isOk = phi i1 [ false, %
  ; CHECK: ], [ true, %L2 ]
; CHECK: br
; CHECK: :
; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK: ret void
}

declare void @_Z18work_group_barrierj(i32)

; DEBUGIFY-NOT: WARNING
