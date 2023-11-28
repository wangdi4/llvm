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
;;      3. Kernel "main" loads "%z" value from offset 8 in the special buffer after calling "foo".
;;      4. function "foo" contains no more barrier/dummy_barrier. instructions
;;      5. function "foo" stores "%x" value to offset 8 in the special buffer before ret.
;;      5.1. This store of value "%x" happens in a new created loop over all WIs
;;*****************************************************************************

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"

target triple = "x86_64-pc-win32"
; CHECK: @main
define void @main(i64 %x) nounwind !no_barrier_path !1 {
L1:
  call void @dummy_barrier.()
  %lid = call i64 @_Z12get_local_idj(i32 0)
  %y = xor i64 %x, %lid
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 1)
  %z = call i64 @foo(i64 %x)
  br label %L3
L3:
  call void @dummy_barrier.()
  %w = and i64 %z, %z
  br label %L4
L4:
  call void @_Z18work_group_barrierj(i32 1)
  ret void
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: xor
; CHECK: call i64 @foo
; CHECK: br label %
;;;; TODO: add regular expression for the below values.

; CHECK: SyncBB{{[0-9]*}}:                                          ; preds = %Dispatch{{[0-9]*}}, %L3
; CHECK: %SBIndex = load i64, ptr %pCurrSBIndex
; CHECK: %SB_LocalId_Offset = add nuw i64 %SBIndex, 8
; CHECK: [[GEP0:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 %SB_LocalId_Offset
; CHECK: %loadedValue = load i64, ptr [[GEP0]]
; CHECK: [[SB_LocalId_Offset2:%.*]] = add nuw i64 %SBIndex, 0
; CHECK: [[GEP1:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[SB_LocalId_Offset2]]
; CHECK: store i64 %loadedValue, ptr [[GEP1]]
; CHECK: [[SB_LocalId_Offset5:%.*]] = add nuw i64 %SBIndex, 0
; CHECK: [[GEP2:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[SB_LocalId_Offset5]]
; CHECK: [[loadedValue7:%.*]] = load i64, ptr [[GEP2]]
; CHECK: %w = and i64 [[loadedValue7]], [[loadedValue7]]
; CHECK: br label %L4
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: ret
}

; CHECK: @foo
define i64 @foo(i64 %x) nounwind {
L1:
  call void @dummy_barrier.()
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 2)
  ret i64 %x
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: LoopBB:
; CHECK: %SBIndex1 = load i64, ptr %pCurrSBIndex
; CHECK: %SB_LocalId_Offset = add nuw i64 %SBIndex1, 8
; CHECK: [[GEP0:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 %SB_LocalId_Offset
; CHECK: store i64 %x, ptr [[GEP0]]
; CHECK: %LocalId_0 = load i64, ptr %pLocalId_0
; CHECK: [[LocalId:%.*]] = add nuw i64 %LocalId_0, 1
; CHECK: store i64 [[LocalId]], ptr %pLocalId_0
; CHECK: [[ICMP:%.*]] = icmp ult i64 [[LocalId]], %LocalSize_0
; CHECK: br i1 [[ICMP]], label %Dispatch, label %LoopEnd_0
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: ret i64 %x
}

declare void @_Z18work_group_barrierj(i32)
declare i64 @_Z12get_local_idj(i32)
declare void @dummy_barrier.()

!sycl.kernels = !{!0}

!0 = !{ptr @main}
!1 = !{i1 false}

;; barrier key values
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %pCurrBarrier = alloca i32, align 4
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %pCurrSBIndex = alloca i64, align 8
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %pLocalIds = alloca [3 x i64], align 8
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %pSB = call ptr @get_special_buffer.()
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %LocalSize_0 = call i64 @_Z14get_local_sizej(i32 0)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %LocalSize_1 = call i64 @_Z14get_local_sizej(i32 1)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %LocalSize_2 = call i64 @_Z14get_local_sizej(i32 2)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %pCurrBarrier = alloca i32, align 4
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %pCurrSBIndex = alloca i64, align 8
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %pLocalIds = alloca [3 x i64], align 8
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %pSB = call ptr @get_special_buffer.()
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %LocalSize_0 = call i64 @_Z14get_local_sizej(i32 0)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %LocalSize_1 = call i64 @_Z14get_local_sizej(i32 1)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %LocalSize_2 = call i64 @_Z14get_local_sizej(i32 2)
; DEBUGIFY-NOT: WARNING
