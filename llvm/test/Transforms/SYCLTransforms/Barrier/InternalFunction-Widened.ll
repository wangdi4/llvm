; RUN: opt -passes=sycl-kernel-barrier -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-barrier -S < %s | FileCheck %s
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
define void @main(i32 %x) nounwind !vectorized_width !1 !no_barrier_path !2 {
L1:
  call void @dummy_barrier.()
  %lid = call i32 @_Z12get_local_idj(i32 0)
  %y = xor i32 %x, %lid
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 1)
; CHECK-LABEL: CallBB:
; CHECK: [[LID:%.*]] = load i32, ptr %pLocalId_0, align 4
; CHECK-NEXT: [[INC:%.*]] = add nuw i32 [[LID]], 8
; CHECK-NEXT: store i32 [[INC]], ptr %pLocalId_0, align 4
  call void @foo(i32 %y)
  br label %L3
L3:
  call void @dummy_barrier.()
  ret void
}

define void @foo(i32 %x) #0 nounwind {
L1:
  call void @dummy_barrier.()
  %y = xor i32 %x, %x
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 2)
; CHECK: [[LID:%.*]] = load i32, ptr %pLocalId_0, align 4
; CHECK-NEXT: [[INC:%.*]] = add nuw i32 [[LID]], 8
; CHECK-NEXT: store i32 [[INC]], ptr %pLocalId_0, align 4
  ret void
}

declare void @_Z18work_group_barrierj(i32)
declare i32 @_Z12get_local_idj(i32)
declare void @dummy_barrier.()

attributes #0 = { "widened-size"="8" }

!sycl.kernels = !{!0}

!0 = !{ptr @main}
!1 = !{i32 8}
!2 = !{i1 false}

;; barrier key values
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %pCurrBarrier = alloca i32, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %pCurrSBIndex = alloca i32, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %pLocalIds = alloca [3 x i32], align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %pSB = call ptr @get_special_buffer.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %LocalSize_0 = call i32 @_Z14get_local_sizej(i32 0)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %LocalSize_1 = call i32 @_Z14get_local_sizej(i32 1)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %LocalSize_2 = call i32 @_Z14get_local_sizej(i32 2)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %pCurrBarrier = alloca i32, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %pCurrSBIndex = alloca i32, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %pLocalIds = alloca [3 x i32], align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %pSB = call ptr @get_special_buffer.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %LocalSize_0 = call i32 @_Z14get_local_sizej(i32 0)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %LocalSize_1 = call i32 @_Z14get_local_sizej(i32 1)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %LocalSize_2 = call i32 @_Z14get_local_sizej(i32 2)

; DEBUGIFY-NOT: WARNING
