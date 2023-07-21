; RUN: opt -passes=sycl-kernel-barrier -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-barrier -S < %s | FileCheck %s
;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and function "foo",
;;           which contains get_local_id() and returns void.
;; The expected result:
;;      1. Kernel "main" contains no more barrier/dummy_barrier. instructions
;;      2. Kernel "main" is still calling function "foo"
;;      3. function "foo" contains no more barrier/dummy_barrier. instructions
;;      4. function "foo" calls get_curr_wi exactly once
;;      5. function "foo" calls get_new_local_id.() instead of get_local_id
;;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"

; CHECK: @main
define void @main() nounwind !no_barrier_path !1 {
L1:
  call void @dummy_barrier.()
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 1)
  call void @foo(i32 0)
  br label %L3
L3:
  call void @dummy_barrier.()
  ret void
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: call void @foo(i32 0, ptr noalias %pLocalIds)
; CHECK: br label %
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: ret
}

define void @foo(i32 %x) nounwind {
; CHECK: define void @foo(i32 %x, ptr noalias %local.ids)
L1:
  call i32 @_Z12get_local_idj(i32 0)
  br label %L2
L2:
  call i32 @_Z13get_global_idj(i32 0)
  ret void

; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: %BaseGlobalId_0 = call i32 @get_base_global_id.(i32 0)
; CHECK: [[GEP:%.*]] = getelementptr inbounds [3 x i32], ptr %local.ids, i64 0, i32 0
; CHECK: %LocalId_0 = load i32, ptr [[GEP]]
; CHECK: br label %L2
; CHECK: L2:                                               ; preds = %L1
; CHECK: %LocalId_01 = load i32, ptr [[GEP]]
; CHECK: %GlobalID_0 = add i32 %LocalId_01, %BaseGlobalId_0
; CHECK: ret
}

declare void @_Z18work_group_barrierj(i32)
declare void @dummy_barrier.()
declare i32 @_Z12get_local_idj(i32)
declare i32 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @main}
!1 = !{i1 false}

;; barrier key values
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %pCurrBarrier = alloca i32, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %pCurrSBIndex = alloca i32, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %pLocalIds = alloca [3 x i32], align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %pSB = call ptr @get_special_buffer.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %LocalSize_0 = call i32 @_Z14get_local_sizej(i32 0)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %LocalSize_1 = call i32 @_Z14get_local_sizej(i32 1)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %LocalSize_2 = call i32 @_Z14get_local_sizej(i32 2)
;; get_global_id resolve
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %BaseGlobalId_0 = call i32 @get_base_global_id.(i32 0)
; DEBUGIFY-NOT: WARNING
