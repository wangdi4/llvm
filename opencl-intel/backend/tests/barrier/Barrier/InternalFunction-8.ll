; RUN: opt -B-Barrier -verify -S < %s | FileCheck %s
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
; CHECK: call void @foo(i32 0, [3 x i32]* %pLocalIds)
; CHECK: br label %
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
; CHECK: ret
}

define void @foo(i32 %x) nounwind {
; CHECK: define void @foo(i32 %x, [3 x i32]* noalias %pLocalIdValues)
L1:
  call i32 @_Z12get_local_idj(i32 0)
  br label %L2
L2:
  call i32 @_Z13get_global_idj(i32 0)
  ret void

; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
; CHECK: %BaseGlobalId_0 = call i32 @get_base_global_id.(i32 0)
; CHECK: %pLocalId_0 = getelementptr inbounds [3 x i32]* %pLocalIdValues, i32 0, i32 0
; CHECK: %LocalId_0 = load i32* %pLocalId_0
; CHECK: br label %L2
; CHECK: L2:                                               ; preds = %L1
; CHECK: %LocalId_01 = load i32* %pLocalId_0
; CHECK: %GlobalID_0 = add i32 %LocalId_01, %BaseGlobalId_0
; CHECK: ret
}

declare void @_Z7barrierj(i32)
declare void @dummybarrier.()
declare i32 @_Z12get_local_idj(i32)
declare i32 @_Z13get_global_idj(i32)

!opencl.kernels = !{!0}
!opencl.build.options = !{}

!0 = !{void ()* @main, !1, !1, !"", !"int", !"opencl_main_locals_anchor", !2, !3, !4, !5, !""}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{i32 0}
!3 = !{i32 3}
!4 = !{!"int"}
!5 = !{!"x"}
