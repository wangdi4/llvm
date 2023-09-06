; RUN: opt -passes=sycl-kernel-barrier -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-barrier -S < %s | FileCheck %s
;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and function "foo",
;;           which contains barrier itself and returns i32 value type,
;;           and receives uniform value "%x" that does not cross barrier.
;; The expected result:
;;      1. Kernel "main" contains no more barrier/dummy_barrier. instructions
;;      2. Kernel "main" is still calling function "foo"
;;      3. Kernel "main" loads "%z" value from offset 4 in the special buffer after calling "foo".
;;      4. function "foo" contains no more barrier/dummy_barrier. instructions
;;      5. function "foo" stores "%x" value to offset 4 in the special buffer before ret.
;;      5.1. This store of value "%x" happens in a new created loop over all WIs
;;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %x) nounwind !no_barrier_path !1 {
L1:
  call void @dummy_barrier.()
  %lid = call i32 @_Z12get_local_idj(i32 0)
  %y = xor i32 %x, %lid
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 1)
  %z = call i32 @foo(i32 %x)
  br label %L3
L3:
  call void @dummy_barrier.()
  %w = and i32 %z, %z
  br label %L4
L4:
  call void @_Z18work_group_barrierj(i32 1)
  ret void
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: xor
; CHECK-NOT: store i32 %x
; CHECK: call i32 @foo
; CHECK: br label %
; CHECK: SyncBB{{[0-9]*}}:                                          ; preds = %Dispatch{{[0-9]*}}, %L3
; CHECK: [[SBIndex:%.*]] = load i32, ptr %pCurrSBIndex
; CHECK: [[SB_LocalId_Offset:%.*]] = add nuw i32 [[SBIndex]], 4
; CHECK: [[pSB_LocalId:%.*]] = getelementptr inbounds i8, ptr %pSB, i32 [[SB_LocalId_Offset]]
; CHECK: %loadedValue = load i32, ptr [[pSB_LocalId]]
; CHECK: [[SB_LocalId_Offset2:%.*]] = add nuw i32 [[SBIndex]], 0
; CHECK: [[pSB_LocalId2:%.*]] = getelementptr inbounds i8, ptr %pSB, i32 [[SB_LocalId_Offset2]]
; CHECK: store i32 %loadedValue, ptr [[pSB_LocalId2]]
; CHECK: [[SB_LocalId_Offset5:%.*]] = add nuw i32 [[SBIndex]], 0
; CHECK: [[pSB_LocalId5:%.*]] = getelementptr inbounds i8, ptr %pSB, i32 [[SB_LocalId_Offset5]]
; CHECK: [[loadedValue7:%.*]] = load i32, ptr [[pSB_LocalId5]]
; CHECK: %w = and i32 [[loadedValue7]], [[loadedValue7]]
; CHECK: br label %L4
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: ret
}

; CHECK: @foo
define i32 @foo(i32 %x) nounwind {
L1:
  call void @dummy_barrier.()
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 2)
  ret i32 %x
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
; The return BB
; CHECK: LoopBB:                                           ; preds = %Dispatch, %SyncBB0
; CHECK: [[SBIndex1:%.*]] = load i32, ptr %pCurrSBIndex
; CHECK: [[SB_LocalId_Offset:%.*]] = add nuw i32 [[SBIndex1]], 4
; CHECK: [[pSB_LocalId:%.*]] = getelementptr inbounds i8, ptr %pSB, i32 [[SB_LocalId_Offset]]
; CHECK: store i32 %x, ptr [[pSB_LocalId]]
; CHECK: %LocalId_0 = load i32, ptr %pLocalId_0
; CHECK: [[LocalID:%.*]] = add nuw i32 %LocalId_0, 1
; CHECK: store i32 [[LocalID]], ptr %pLocalId_0
; CHECK: [[ICMP:%.*]] = icmp ult i32 [[LocalID]], %LocalSize_0
; CHECK: br i1 [[ICMP]], label %Dispatch, label %LoopEnd_0
; CHECK: ret i32 %x
}

declare void @_Z18work_group_barrierj(i32)
declare i32 @_Z12get_local_idj(i32)
declare void @dummy_barrier.()

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
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %pCurrBarrier = alloca i32, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %pCurrSBIndex = alloca i32, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %pLocalIds = alloca [3 x i32], align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %pSB = call ptr @get_special_buffer.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %LocalSize_0 = call i32 @_Z14get_local_sizej(i32 0)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %LocalSize_1 = call i32 @_Z14get_local_sizej(i32 1)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %LocalSize_2 = call i32 @_Z14get_local_sizej(i32 2)
; DEBUGIFY-NOT: WARNING
