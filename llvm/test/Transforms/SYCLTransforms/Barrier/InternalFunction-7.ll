; RUN: opt -passes=sycl-kernel-barrier -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-barrier -S < %s | FileCheck %s

;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and function "foo",
;;           which contains barrier itself and returns void,
;;           and receives non-uniform alloca address value "%x" (that also cross barrier).
;; The expected result:
;;      1. Kernel "main" contains no more barrier/dummy_barrier. instructions
;;      2. Kernel "main" stores "%x" address value to offset 0 in the special buffer before calling "foo".
;;      3. Kernel "main" is still calling function "foo"
;;      4. function "foo" contains no more barrier/dummy_barrier. instructions
;;      5. function "foo" loads "%x" address value from offset 4 in the special buffer before loading from it.
;;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"

; CHECK: @main
define void @main() nounwind !no_barrier_path !1 {
L1:
  call void @dummy_barrier.()
  %x = alloca i32
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 1)
  call void @foo(ptr %x)
  br label %L3
L3:
  call void @dummy_barrier.()
  ret void
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
;;;; TODO: add regular expression for the below values.
; CHECK-LABEL: SyncBB{{[0-9]*}}:
; CHECK: [[LOAD:%[0-9]+]] = load ptr, ptr %x.addr
; CHECK:   br label %L2
; CHECK: L2:
; CHECK:   %SBIndex = load i32, ptr %pCurrSBIndex
; CHECK:   %SB_LocalId_Offset = add nuw i32 %SBIndex, 4
; CHECK:   [[GEP:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i32 %SB_LocalId_Offset
; CHECK:   store ptr [[LOAD]], ptr [[GEP]]
; CHECK:   br label %CallBB
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: call void @foo
; CHECK: br label %
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: ret
}

; CHECK: @foo
define void @foo(ptr %x) nounwind {
L1:
  call void @dummy_barrier.()
  load i32, ptr %x
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 2)
  ret void
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: SyncBB1:
; CHECK:   %SBIndex = load i32, ptr %pCurrSBIndex
; CHECK:   %SB_LocalId_Offset = add nuw i32 %SBIndex, 4
; CHECK:   [[GEP0:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i32 %SB_LocalId_Offset
; CHECK:   %loadedValue = load ptr, ptr [[GEP0]]
; CHECK:   load i32, ptr %loadedValue
; CHECK:   br label %L2
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: ret
}

declare void @_Z18work_group_barrierj(i32)
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
