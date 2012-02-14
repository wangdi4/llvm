; RUN: llvm-as %s -o %t.bc
; RUN: opt -B-Barrier -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and function "foo",
;;           which contains barrier itself and returns i32 value type,
;;           and receives uniform value "%x" that does not cross barrier.
;; The expected result:
;;      1. Kernel "main" contains no more barrier/dummybarrier instructions
;;      2. Kernel "main" replaced the call to function "foo" with a call to "foo_New"
;;      3. function "foo" contains no more barrier/dummybarrier instructions
;;      4. function "foo_New" receives these parameters (i64 %x, i64 %offset)
;;      5. function "foo_New" contains no more barrier/dummybarrier instructions
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"

target triple = "x86_64-pc-win32"
; CHECK: @main
define void @main(i64 %x) nounwind {
L1:
  call void @dummybarrier.()
  %lid = call i64 @get_local_id(i32 0)
  %y = xor i64 %x, %lid
  br label %L2
L2:
  call void @barrier(i64 1)
  %z = call i64 @foo(i64 %x)
  br label %L3
L3:
  call void @dummybarrier.()
  %w = and i64 %z, %z
  br label %L4
L4:
  call void @barrier(i64 1)
  ret void
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @barrier
; CHECK: xor
; CHECK: call i64 @foo_New
; CHECK: br label %
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @barrier
; CHECK: ret
}

; CHECK: @foo
define i64 @foo(i64 %x) nounwind {
L1:
  call void @dummybarrier.()
  br label %L2
L2:
  call void @barrier(i64 2)
  ret i64 %x
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @barrier
; CHECK: ret i64 %x
}

; CHECK: @foo_New(i64 %x, i64 %offset)
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @barrier
; CHECK: ret i64 %x

declare void @barrier(i64)
declare i64 @get_local_id(i32)
declare void @dummybarrier.()

!opencl.kernels = !{!0}
!opencl.build.options = !{}

!0 = metadata !{void (i64)* @main, metadata !1, metadata !1, metadata !"", metadata !"int", metadata !"opencl_main_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !""}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 0}
!3 = metadata !{i32 3}
!4 = metadata !{metadata !"int"}
!5 = metadata !{metadata !"x"}
