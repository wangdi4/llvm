; RUN: llvm-as %s -o %t.bc
; RUN: opt -B-RemoveDuplication -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the RemoveDuplicationBarrier pass
;; The case: function "main" with the following synchronize instruction sequence
;;           dummybarrier
;;           barrier(GLOBAL_MEM_FENCE)
;; The expected result:
;;      1. The following synchronize instruction sequence
;;         a. dummybarrier
;;         b. barrier(GLOBAL_MEM_FENCE)
;;*****************************************************************************

; CHECK: @main
define void @main() {
  call void @dummybarrier.()
  call void @barrier(i32 2)

  ret void
; CHECK: @dummybarrier.()
; CHECK: @barrier(i32 2)
; CHECK: ret
}

declare void @barrier(i32)
declare void @dummybarrier.()