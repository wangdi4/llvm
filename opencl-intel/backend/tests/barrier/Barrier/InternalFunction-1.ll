; RUN: llvm-as %s -o %t.bc
; RUN: opt -B-Barrier -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and the non-uniform value "%y"
;;           that crosses the barrier instruction and is an input to function "foo",
;;           which contains barrier itself and returns void.
;; The expected result:
;;      1. Kernel "main" contains no more barrier/dummybarrier instructions
;;      2. Kernel "main" replaced the call to function "foo" with a call to "foo_New"
;;      3. function "foo" contains no more barrier/dummybarrier instructions
;;      4. function "foo_New" receives these parameters (i32 %x, i32 %offset)
;;      5. function "foo_New" contains no more barrier/dummybarrier instructions
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f3
2:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %x) nounwind {
L1:
  call void @dummybarrier.()
  %lid = call i32 @get_local_id(i32 0)
  %y = xor i32 %x, %lid
  br label %L2
L2:
  call void @barrier(i32 1)
  call void @foo(i32 %y)
  br label %L3
L3:
  call void @dummybarrier.()
  ret void
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @barrier
; CHECK: xor
; CHECK: call void @foo_New
; CHECK: br label %
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @barrier
; CHECK: ret
}

; CHECK: @foo
define void @foo(i32 %x) nounwind {
L1:
  call void @dummybarrier.()
  %y = xor i32 %x, %x
  br label %L2
L2:
  call void @barrier(i32 2)
  ret void
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @barrier
; CHECK: xor
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @barrier
; CHECK: ret
}

; CHECK: @foo_New(i32 %x, i32 %offset)
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @barrier
; CHECK: xor
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @barrier
; CHECK: ret

declare void @barrier(i32)
declare i32 @get_local_id(i32)
declare void @dummybarrier.()

!opencl.kernels = !{!0}
!opencl.build.options = !{}

!0 = metadata !{void (i32)* @main, metadata !1, metadata !1, metadata !"", metadata !"int", metadata !"opencl_main_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !""}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 0}
!3 = metadata !{i32 3}
!4 = metadata !{metadata !"int"}
!5 = metadata !{metadata !"x"}
