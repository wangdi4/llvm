; RUN: llvm-as %s -o %t.bc
; RUN: opt -B-RemoveDuplication -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the RemoveDuplicationBarrier pass
;; The case: function "main" with the following synchronize instruction sequence
;;           fiber
;;           fiber
;; The expected result:
;;      1. The following synchronize instruction sequence
;;         a. fiber
;;         b. [but no other fiber]
;;*****************************************************************************

; CHECK: @main
define void @main() {
  call void @fiber.()
  call void @fiber.()

  ret void
; CHECK: @fiber.()
; CHECK-NOT: @fiber.
; CHECK: ret
}

declare void @fiber.()