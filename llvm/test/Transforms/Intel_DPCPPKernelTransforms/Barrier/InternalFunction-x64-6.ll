; RUN: opt -passes=dpcpp-kernel-barrier -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-barrier -S < %s | FileCheck %s
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
; CHECK: [[SBINDEX1:%SBIndex[0-9]*]] = load i64, i64* %pCurrSBIndex
; CHECK: [[SB_LOCALID_OFFSET1:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[SBINDEX1]], 16
; CHECK: [[GEP1:%[0-9]+]] = getelementptr inbounds i8, i8* %pSB, i64 [[SB_LOCALID_OFFSET1]]
; CHECK: [[PSB_LOCALID1:%pSB_LocalId[0-9]*]] = bitcast i8* [[GEP1]] to i64*
; CHECK: [[SBINDEX2:%SBIndex[0-9]*]] = load i64, i64* %pCurrSBIndex
; CHECK: [[SB_LOCALID_OFFSET2:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[SBINDEX2]], 0
; CHECK: [[GEP2:%[0-9]+]] = getelementptr inbounds i8, i8* %pSB, i64 [[SB_LOCALID_OFFSET2]]
; CHECK: [[PSB_LOCALID2:%pSB_LocalId[0-9]*]] = bitcast i8* [[GEP2]] to i64*
; CHECK: [[LOADED_VALUE2:%loadedValue[0-9]*]] = load i64, i64* [[PSB_LOCALID2]]
; CHECK: store i64 [[LOADED_VALUE2]], i64* [[PSB_LOCALID1]]
; CHECK: br label %CallBB{{[0-9]*}}
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: call i64 @foo
; CHECK: br label %
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: SyncBB3:                                          ; preds = %Dispatch{{[0-9]*}}, %L2A
; CHECK: [[SBINDEX3:%SBIndex[0-9]*]] = load i64, i64* %pCurrSBIndex
; CHECK: [[SB_LOCALID_OFFSET3:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[SBINDEX3]], 24
; CHECK: [[GEP3:%[0-9]+]] = getelementptr inbounds i8, i8* %pSB, i64 [[SB_LOCALID_OFFSET3]]
; CHECK: [[PSB_LOCALID3:%pSB_LocalId[0-9]*]] = bitcast i8* [[GEP3]] to i64*
; CHECK: [[LOADED_VALUE3:%loadedValue[0-9]*]] = load i64, i64* [[PSB_LOCALID3]]
; CHECK: [[SBINDEX4:%SBIndex[0-9]*]] = load i64, i64* %pCurrSBIndex
; CHECK: [[SB_LOCALID_OFFSET4:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[SBINDEX4]], 8
; CHECK: [[GEP4:%[0-9]+]] = getelementptr inbounds i8, i8* %pSB, i64 [[SB_LOCALID_OFFSET4]]
; CHECK: [[PSB_LOCALID4:%pSB_LocalId[0-9]*]] = bitcast i8* [[GEP4]] to i64*
; CHECK: store i64 [[LOADED_VALUE3]], i64* [[PSB_LOCALID4]]
; CHECK: br label %L3
;;;; TODO: add regular expression for the below values.
; CHECK: L3:
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: [[SBINDEX4:%SBIndex[0-9]*]] = load i64, i64* %pCurrSBIndex
; CHECK: [[SB_LOCALID_OFFSET4:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[SBINDEX4]], 16
; CHECK: [[GEP4:%[0-9]+]] = getelementptr inbounds i8, i8* %pSB, i64 [[SB_LOCALID_OFFSET4]]
; CHECK: [[PSB_LOCALID4:%pSB_LocalId[0-9]*]] = bitcast i8* [[GEP4]] to i64*
; CHECK: [[SBINDEX5:%SBIndex[0-9]*]] = load i64, i64* %pCurrSBIndex
; CHECK: [[SB_LOCALID_OFFSET5:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[SBINDEX5]], 8
; CHECK: [[GEP5:%[0-9]+]] = getelementptr inbounds i8, i8* %pSB, i64 [[SB_LOCALID_OFFSET5]]
; CHECK: [[PSB_LOCALID5:%pSB_LocalId[0-9]*]] = bitcast i8* [[GEP5]] to i64*
; CHECK: [[LOADED_VALUE6:%loadedValue[0-9]*]] = load i64, i64* [[PSB_LOCALID5]]
; CHECK: store i64 [[LOADED_VALUE6]], i64* [[PSB_LOCALID4]]
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
; CHECK: %SBIndex = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset = add nuw i64 %SBIndex, 16
; CHECK: %0 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset
; CHECK: %pSB_LocalId = bitcast i8* %0 to i64*
; CHECK: %loadedValue = load i64, i64* %pSB_LocalId
; CHECK: %y = xor i64 %loadedValue, %loadedValue
; CHECK: %SBIndex1 = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset2 = add nuw i64 %SBIndex1, 24
; CHECK: %1 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset2
; CHECK: %pSB_LocalId3 = bitcast i8* %1 to i64*
; CHECK: store i64 %y, i64* %pSB_LocalId3
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

!0 = !{void (i64)* @main}
!1 = !{i1 false}

;; barrier key values
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %pCurrBarrier = alloca i32, align 4
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %pCurrSBIndex = alloca i64, align 8
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %pLocalIds = alloca [3 x i64], align 8
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %pSB = call i8* @get_special_buffer.()
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %LocalSize_0 = call i64 @_Z14get_local_sizej(i32 0)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %LocalSize_1 = call i64 @_Z14get_local_sizej(i32 1)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main -- %LocalSize_2 = call i64 @_Z14get_local_sizej(i32 2)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %pCurrBarrier = alloca i32, align 4
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %pCurrSBIndex = alloca i64, align 8
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %pLocalIds = alloca [3 x i64], align 8
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %pSB = call i8* @get_special_buffer.()
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %LocalSize_0 = call i64 @_Z14get_local_sizej(i32 0)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %LocalSize_1 = call i64 @_Z14get_local_sizej(i32 1)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %LocalSize_2 = call i64 @_Z14get_local_sizej(i32 2)
;; argument
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %loadedValue = load i64, i64* %pSB_LocalId, align 8
; DEBUGIFY-NOT: WARNING
