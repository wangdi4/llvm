; RUN: opt -B-RemoveDuplication -verify -S < %s | FileCheck %s
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

;;*****************************************************************************
;; This test checks the RemoveDuplicationBarrier pass
;; The case: function "main" with the following synchronize instruction sequence
;;           dummybarrier
;;           fiber
;;           barrier(GLOBAL_MEM_FENCE)
;; The expected result:
;;      1. The following synchronize instruction sequence
;;         a. dummybarrier
;;         b. barrier(GLOBAL_MEM_FENCE)
;;         c. [but no fiber]
;;*****************************************************************************

; CHECK: @main
define void @main() {
  call void @dummybarrier.()
  call void @fiber.()
  call void @_Z7barrierj(i32 2)

  ret void
; CHECK-NOT: @fiber.
; CHECK: @dummybarrier.
; CHECK: @_Z7barrierj(i32 2)
; CHECK-NOT: @fiber.
; CHECK: ret
}

declare void @dummybarrier.()
declare void @_Z7barrierj(i32)
declare void @fiber.()
