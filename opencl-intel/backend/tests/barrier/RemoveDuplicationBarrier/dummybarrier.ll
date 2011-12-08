; RUN: llvm-as %s -o %t.bc
; RUN: opt -B-RemoveDuplication -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the RemoveDuplicationBarrier pass
;; The case: function "main" with the following synchronize instruction sequence
;;           dummybarrier
;;           dummybarrier
;; The expected result:
;;      1. The following synchronize instruction sequence
;;         a. dummybarrier
;;         b. [but no other dummybarrier]
;;*****************************************************************************

; CHECK: @main
define void @main() {
  call void @dummybarrier.()
  call void @dummybarrier.()

  ret void
; CHECK: @dummybarrier.()
; CHECK-NOT: @dummybarrier.()
; CHECK: ret
}

declare void @dummybarrier.()