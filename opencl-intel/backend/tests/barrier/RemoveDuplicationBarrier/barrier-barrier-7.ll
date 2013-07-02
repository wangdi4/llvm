; RUN: llvm-as %s -o %t.bc
; RUN: opt -B-RemoveDuplication -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the RemoveDuplicationBarrier pass
;; The case: function "main" with the following synchronize instruction sequence
;;           barrier(LOCAL_MEM_FENCE | GLOBAL_MEM_FENCE)
;;           barrier(GLOBAL_MEM_FENCE)
;; The expected result:
;;      1. The following synchronize instruction sequence
;;         a. barrier(GLOBAL_MEM_FENCE)
;;         b. [but no other barrier]
;;*****************************************************************************

; CHECK: @main
define void @main() {
  call void @_Z7barrierj(i32 3)
  call void @_Z7barrierj(i32 2)

  ret void
; CHECK-NOT: @_Z7barrierj
; CHECK: @_Z7barrierj(i32 2)
; CHECK-NOT: @_Z7barrierj
; CHECK: ret
}

declare void @_Z7barrierj(i32)