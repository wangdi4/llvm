; RUN: opt -passes=sycl-kernel-barrier -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-barrier -S < %s | FileCheck %s
;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and the non-uniform value "%y"
;;           that crosses the barrier instruction and is an input to function "foo",
;;           which contains barrier itself and returns void.
;;           kernel main also calls same "foo" function with uniform value "%x"
;; The expected result:
;;      1. Kernel "main" contains no more barrier/dummy_barrier. instructions
;;      2. Kernel "main" stores "%y" value to offset 8 in the special buffer before calling "foo".
;;      3. Kernel "main" is still calling function "foo"
;;      4. Kernel "main" stores "%x" value to offset 8 in the special buffer before calling "foo".
;;      5. Kernel "main" is still calling function "foo"
;;      6. function "foo" contains no more barrier/dummy_barrier. instructions
;;      7. function "foo" loads "%x" value from offset 8 in the special buffer before xor.
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
  call void @foo(i64 %y)
  br label %L2A
L2A:
  call void @dummy_barrier.()
  br label %L3
L3:
  call void @_Z18work_group_barrierj(i32 1)
  call void @foo(i64 %x)
  br label %L3A
L3A:
  call void @dummy_barrier.()
  ret void
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: xor
; CHECK: br label %
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: L2:                                               ; preds = %SyncBB{{[0-9]*}}
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: [[SBINDEX1:%SBIndex[0-9]*]] = load i64, ptr %pCurrSBIndex
; CHECK: [[SB_LOCALID_OFFSET1:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[SBINDEX1]], 8
; CHECK: [[GEP1:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[SB_LOCALID_OFFSET1]]
; CHECK: [[SB_LOCALID_OFFSET2:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[SBINDEX1]], 0
; CHECK: [[GEP2:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[SB_LOCALID_OFFSET2]]
; CHECK: [[LOADED_VALUE:%loadedValue[0-9]*]] = load i64, ptr [[GEP2]]
; CHECK: store i64 [[LOADED_VALUE]], ptr [[GEP1]]
; CHECK: br label %CallBB{{[0-9]*}}
; CHECK: call void @foo
; CHECK: br label %
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: L3:
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: [[SBINDEX3:%SBIndex[0-9]*]] = load i64, ptr %pCurrSBIndex
; CHECK: [[SB_LOCALID_OFFSET3:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[SBINDEX3]], 8
; CHECK: [[GEP3:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[SB_LOCALID_OFFSET3]]
; CHECK: store i64 %x, ptr [[GEP3]]
; CHECK: br label %CallBB{{[0-9]*}}
; CHECK: call void @foo
; CHECK: br label %
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: ret
}

; CHECK: @foo
define void @foo(i64 %x) nounwind {
L1:
  call void @dummy_barrier.()
  %y = xor i64 %x, %x
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 2)
  ret void
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: SyncBB1:      
; CHECK: %SBIndex = load i64, ptr %pCurrSBIndex
; CHECK: %SB_LocalId_Offset = add nuw i64 %SBIndex, 8
; CHECK: [[GEP0:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 %SB_LocalId_Offset
; CHECK: %loadedValue = load i64, ptr [[GEP0]]
; CHECK: %y = xor i64 %loadedValue, %loadedValue
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
