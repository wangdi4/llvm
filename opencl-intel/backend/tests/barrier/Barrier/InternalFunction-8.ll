; RUN: llvm-as %s -o %t.bc
; RUN: opt -B-Barrier -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; XFAIL: *

;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and function "foo",
;;           which contains get_local_id() and returns void.
;; The expected result:
;;      1. Kernel "main" contains no more barrier/dummybarrier instructions
;;      2. Kernel "main" is still calling function "foo"
;;      3. function "foo" contains no more barrier/dummybarrier instructions
;;      4. function "foo" calls get_curr_wi exactly once
;;      5. function "foo" calls get_new_local_id.() instead of get_local_id
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"

; CHECK: @main
define void @main() nounwind {
L1:
  call void @dummybarrier.()
  br label %L2
L2:
  call void @_Z7barrierj(i32 1)
  call void @foo(i32 0)
  br label %L3
L3:
  call void @dummybarrier.()
  ret void
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
; CHECK: call void @foo
; CHECK: br label %
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
; CHECK: ret
}

; CHECK: @foo
define void @foo(i32 %x) nounwind {
L1:
  call i32 @_Z12get_local_idj(i32 0)
  br label %L2
L2:
  call i32 @_Z12get_local_idj(i32 0)
  ret void
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
; CHECK: call i32* @get_curr_wi.()
; CHECK-NOT: call i32* @get_curr_wi.()
; CHECK: call i32 @get_new_local_id.(i32 0,
; CHECK: call i32 @get_new_local_id.(i32 0,
; CHECK: ret
}

declare void @_Z7barrierj(i32)
declare void @dummybarrier.()
declare i32 @_Z12get_local_idj(i32)

!opencl.kernels = !{!0}
!opencl.build.options = !{}

!0 = metadata !{void ()* @main, metadata !1, metadata !1, metadata !"", metadata !"int", metadata !"opencl_main_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !""}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 0}
!3 = metadata !{i32 3}
!4 = metadata !{metadata !"int"}
!5 = metadata !{metadata !"x"}
