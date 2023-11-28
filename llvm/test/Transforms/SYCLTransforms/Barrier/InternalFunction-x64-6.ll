; RUN: opt -passes=sycl-kernel-barrier -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-barrier -S < %s | FileCheck %s
;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and the non-uniform value "%y"
;;           that crosses the barrier instruction and is an input to function "foo",
;;           which contains barrier itself and returns i64 value.
;;           kernel main also calls same "foo" function with non-uniform returned value value "%r1"
;; The expected result:
;;      1. Kernel "main" contains no more barrier/dummy_barrier. instructions
;;      2. Kernel "main" stores "%y" value to offset 16 in the special buffer before calling "foo".
;;      3. Kernel "main" is still calling function "foo"
;;      4. Kernel "main" loads "%r1" value from offset 24 in the special buffer after calling "foo".
;;      5. Kernel "main" stores "%r1" value to offset 16 in the special buffer before calling "foo".
;;      6. Kernel "main" is still calling function "foo"
;;      7. function "foo" contains no more barrier/dummy_barrier. instructions
;;      8. function "foo" loads "%x" value from offset 16 in the special buffer before xor.
;;      9. function "foo" stores "%y" value to offset 24 in the special buffer before ret.
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
  %r1 = call i64 @foo(i64 %y)
  br label %L2A
L2A:
  call void @dummy_barrier.()
  br label %L3
L3:
  call void @_Z18work_group_barrierj(i32 1)
  %r2 = call i64 @foo(i64 %r1)
  br label %L3B
L3B:
  call void @dummy_barrier.()
  ret void
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: L2:                                               ; preds = %SyncBB{{[0-9]*}}
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: [[SBINDEX1:%SBIndex[0-9]*]] = load i64, ptr %pCurrSBIndex
; CHECK: [[SB_LOCALID_OFFSET1:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[SBINDEX1]], 16
; CHECK: [[GEP1:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[SB_LOCALID_OFFSET1]]
; CHECK: [[SB_LOCALID_OFFSET2:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[SBINDEX1]], 0
; CHECK: [[GEP2:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[SB_LOCALID_OFFSET2]]
; CHECK: [[LOADED_VALUE2:%loadedValue[0-9]*]] = load i64, ptr [[GEP2]]
; CHECK: store i64 [[LOADED_VALUE2]], ptr [[GEP1]]
; CHECK: br label %CallBB{{[0-9]*}}
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: call i64 @foo
; CHECK: br label %
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: SyncBB3:                                          ; preds = %Dispatch{{[0-9]*}}, %L2A
; CHECK: [[SBINDEX2:%SBIndex[0-9]*]] = load i64, ptr %pCurrSBIndex
; CHECK: [[SB_LOCALID_OFFSET3:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[SBINDEX2]], 24
; CHECK: [[GEP3:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[SB_LOCALID_OFFSET3]]
; CHECK: [[LOADED_VALUE3:%loadedValue[0-9]*]] = load i64, ptr [[GEP3]]
; CHECK: [[SB_LOCALID_OFFSET4:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[SBINDEX2]], 8
; CHECK: [[GEP4:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[SB_LOCALID_OFFSET4]]
; CHECK: store i64 [[LOADED_VALUE3]], ptr [[GEP4]]
; CHECK: br label %L3
;;;; TODO: add regular expression for the below values.
; CHECK: L3:
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: [[SBINDEX3:%SBIndex[0-9]*]] = load i64, ptr %pCurrSBIndex
; CHECK: [[SB_LOCALID_OFFSET4:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[SBINDEX3]], 16
; CHECK: [[GEP4:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[SB_LOCALID_OFFSET4]]
; CHECK: [[SB_LOCALID_OFFSET5:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[SBINDEX3]], 8
; CHECK: [[GEP5:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[SB_LOCALID_OFFSET5]]
; CHECK: [[LOADED_VALUE6:%loadedValue[0-9]*]] = load i64, ptr [[GEP5]]
; CHECK: store i64 [[LOADED_VALUE6]], ptr [[GEP4]]
; CHECK: br label %CallBB{{[0-9]*}}
;;;; TODO: add regular expression for the below values.
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: call i64 @foo
; CHECK: br label %
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: ret
}

; CHECK: @foo
define i64 @foo(i64 %x) nounwind {
L1:
  call void @dummy_barrier.()
  %y = xor i64 %x, %x
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 2)
  ret i64 %y
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: SyncBB1:
; CHECK: %SBIndex = load i64, ptr %pCurrSBIndex
; CHECK: %SB_LocalId_Offset = add nuw i64 %SBIndex, 16
; CHECK: [[GEP0:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 %SB_LocalId_Offset
; CHECK: %loadedValue = load i64, ptr [[GEP0]]
; CHECK: %y = xor i64 %loadedValue, %loadedValue
; CHECK: %SB_LocalId_Offset2 = add nuw i64 %SBIndex, 24
; CHECK: [[GEP1:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 %SB_LocalId_Offset2
; CHECK: store i64 %y, ptr [[GEP1]]
; CHECK: br label %L2
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: ret
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
