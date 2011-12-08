; RUN: llvm-as %s -o %t.bc
; RUN: opt -B-RemoveDuplication -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the RemoveDuplicationBarrier pass
;; The case: function "main" with the following synchronize instruction sequence
;;           barrier(GLOBAL_MEM_FENCE)
;;           fiber
;; The expected result:
;;      1. The following synchronize instruction sequence
;;         a. barrier(GLOBAL_MEM_FENCE)
;;         b. [but no fiber]
;;*****************************************************************************

; CHECK: @main
define void @main() {
  call void @barrier(i32 2)
  call void @fiber.()

  ret void
; CHECK: @barrier(i32 2)
; CHECK-NOT: @fiber.
; CHECK: ret
}

declare void @barrier(i32)
declare void @fiber.()