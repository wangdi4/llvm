; RUN: llvm-as %s -o %t.bc
; RUN: opt -B-RemoveDuplication -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; XFAIL: *
;;Don't remove dummyBarrier-any, Barrier pass might fail
;;*****************************************************************************
;; This test checks the RemoveDuplicationBarrier pass
;; The case: function "main" with the following synchronize instruction sequence
;;           dummybarrier
;;           fiber
;;           barrier(LOCAL_MEM_FENCE)
;; The expected result:
;;      1. The following synchronize instruction sequence
;;         a. dummybarrier
;;         b. [but no fiber or barrier]
;;*****************************************************************************

; CHECK: @main
define void @main() {
  call void @dummybarrier.()
  call void @fiber.()
  call void @_Z7barrierj(i32 1)

  ret void
; CHECK-NOT: @_Z7barrierj
; CHECK-NOT: @fiber.
; CHECK: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
; CHECK-NOT: @fiber.
; CHECK: ret
}

declare void @dummybarrier.()
declare void @_Z7barrierj(i32)
declare void @fiber.()