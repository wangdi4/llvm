; Checks barrier pass resolves get_local_id and get_global_id correctly.
; RUN: opt -passes=sycl-kernel-barrier %s -S -o - | FileCheck %s
; RUN: opt -passes=sycl-kernel-barrier %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK-LABEL: define void @main
define void @main(i32 %x) #0 !no_barrier_path !1 {
entry:
  call void @dummy_barrier.()
;CHECK: %BaseGlobalId_0 = call i32 @get_base_global_id.(i32 0)
;CHECK: [[LID:%LocalId_[0-9]*]] = load i32, ptr %pLocalId_0, align 4
;CHECK-NEXT: {{%GlobalID_[0-9]*}} = add i32 [[LID]], %BaseGlobalId_0
;CHECK-NEXT: {{%LocalId_[0-9]*}} = load i32, ptr %pLocalId_0, align 4
;CHECK-NEXT: call void @foo(ptr noalias %pLocalIds)
  %gid = call i32 @_Z13get_global_idj(i32 0)
  %lid = call i32 @_Z12get_local_idj(i32 0)
  call void @foo()
  br label %L
L:
  call void @_Z18work_group_barrierj(i32 1)
  ret void
}

; CHECK-LABEL: define void @foo
define void @foo() {
entry:
;CHECK: %BaseGlobalId_0 = call i32 @get_base_global_id.(i32 0)
;CHECK-NEXT: [[GEP:%.*]] = getelementptr inbounds [3 x i32], ptr %local.ids, i64 0, i32 0
;CHECK-NEXT: [[LID:%LocalId_[0-9]*]] = load i32, ptr [[GEP]], align 4
;CHECK-NEXT: {{%GlobalID_[0-9]*}} = add i32 [[LID]], %BaseGlobalId_0
;CHECK-NEXT: {{%LocalId_[0-9]*}} = load i32, ptr [[GEP]], align 4
  %gid = call i32 @_Z13get_global_idj(i32 0)
  %lid = call i32 @_Z12get_local_idj(i32 0)
  ret void
}

declare void @_Z18work_group_barrierj(i32)
declare i32 @_Z12get_local_idj(i32)
declare i32 @_Z13get_global_idj(i32)
declare void @dummy_barrier.()

attributes #0 = { "no-barrier-path"="false" }

!sycl.kernels = !{!0}
!0 = !{ptr @main}
!1 = !{i1 false}

;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main --  %BaseGlobalId_0 = call i32 @get_base_global_id.(i32 0)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main --  %pCurrBarrier = alloca i32, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main --  %pCurrSBIndex = alloca i32, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main --  %pLocalIds = alloca [3 x i32], align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main --  %pSB = call ptr @get_special_buffer.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main --  %LocalSize_0 = call i32 @_Z14get_local_sizej(i32 0)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main --  %LocalSize_1 = call i32 @_Z14get_local_sizej(i32 1)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main --  %LocalSize_2 = call i32 @_Z14get_local_sizej(i32 2)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo --  %BaseGlobalId_0 = call i32 @get_base_global_id.(i32 0)
;DEBUGIFY-NOT: WARNING
