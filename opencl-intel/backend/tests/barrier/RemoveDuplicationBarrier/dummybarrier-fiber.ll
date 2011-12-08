; RUN: llvm-as %s -o %t.bc
; RUN: opt -B-RemoveDuplication -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the RemoveDuplicationBarrier pass
;; The case: function "main" with the following synchronize instruction sequence
;;           dummybarrier
;;           fiber
;; The expected result:
;;      1. The following synchronize instruction sequence
;;         a. dummybarrier
;;         b. [but no fiber]
;;*****************************************************************************

; CHECK: @main
define void @main() {
  call void @dummybarrier.()
  call void @fiber.()

  ret void
; CHECK: @dummybarrier.()
; CHECK-NOT: @fiber.
; CHECK: ret
}

declare void @fiber.()
declare void @dummybarrier.()