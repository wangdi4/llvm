; RUN: llvm-as %s -o %t.bc
; RUN: opt -B-RemoveDuplication -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the RemoveDuplicationBarrier pass
;; The case: function "main" with the following synchronize instruction sequence
;;           dummybarrier
;;           barrier(LOCAL_MEM_FENCE | GLOBAL_MEM_FENCE)
;; The expected result:
;;      1. The following synchronize instruction sequence
;;         a. dummybarrier
;;         b. barrier(LOCAL_MEM_FENCE | GLOBAL_MEM_FENCE)
;;*****************************************************************************

; CHECK: @main
define void @main() {
  call void @dummybarrier.()
  call void @barrier(i32 3)

  ret void
; CHECK: @dummybarrier.()
; CHECK: @barrier(i32 3)
; CHECK: ret
}

declare void @barrier(i32)
declare void @dummybarrier.()