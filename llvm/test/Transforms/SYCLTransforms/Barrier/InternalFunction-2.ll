; RUN: opt -passes=sycl-kernel-barrier -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-barrier -S < %s | FileCheck %s

;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and the non-uniform value "%y"
;;           that crosses the barrier instruction and is an input to function "foo",
;;           which contains barrier itself and returns i32 value type that is not crossing barrier!
;; The expected result:
;;      1. Kernel "main" contains no more barrier/dummy_barrier. instructions
;;      2. Kernel "main" stores "%y" value to offset 4 in the special buffer before calling "foo".
;;      3. Kernel "main" is still calling function "foo"
;;      4. function "foo" contains no more barrier/dummy_barrier. instructions
;;      5. function "foo" loads "%x" value from offset 4 in the special buffer before xor.
;;      6. function "foo" does not store ret value ("%x") in the special buffer before ret.
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
  %z = call i32 @foo(i32 %y)
  br label %L3
L3:
  call void @dummy_barrier.()
  ret void
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: xor
;;;; TODO: add regular expression for the below values.
; CHECK: L2:
; CHECK:   %SBIndex = load i32, ptr %pCurrSBIndex
; CHECK:   %SB_LocalId_Offset = add nuw i32 %SBIndex, 4
; CHECK:   [[GEP0:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i32 %SB_LocalId_Offset
; CHECK:   [[SB_LocalId_Offset1:%SB_LocalId_Offset[a-zA-Z0-9]+]] = add nuw i32 %SBIndex, 0
; CHECK:   [[GEP1:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i32 [[SB_LocalId_Offset1]]
; CHECK:   %loadedValue = load i32, ptr [[GEP1]]
; CHECK:   store i32 %loadedValue, ptr [[GEP0]]
; CHECK:   br label %CallBB
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: call i32 @foo
; CHECK: br label %
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: ret
}

; CHECK: @foo
define i32 @foo(i32 %x) nounwind {
L1:
  call void @dummy_barrier.()
  %y = xor i32 %x, %x
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 2)
  ret i32 %x
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: SyncBB1:
; CHECK:   [[SBIndex0:%SBIndex[a-zA-Z0-9]+]] = load i32, ptr %pCurrSBIndex
; CHECK:   [[SB_LocalId_Offset0:%SB_LocalId_Offset[a-zA-Z0-9]+]] = add nuw i32 [[SBIndex0]], 4
; CHECK:   [[GEP0:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i32 [[SB_LocalId_Offset0]]
; CHECK:   [[loadedValue0:%loadedValue[a-zA-Z0-9]+]] = load i32, ptr [[GEP0]]
; CHECK:   %y = xor i32 [[loadedValue0]], [[loadedValue0]]
; CHECK:   br label %L2
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: SyncBB0:
; CHECK:   %SBIndex = load i32, ptr %pCurrSBIndex
; CHECK:   %SB_LocalId_Offset = add nuw i32 %SBIndex, 4
; CHECK:   [[GEP0:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i32 %SB_LocalId_Offset
; CHECK:   %loadedValue = load i32, ptr [[GEP0]]
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: ret i32 %loadedValue
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
