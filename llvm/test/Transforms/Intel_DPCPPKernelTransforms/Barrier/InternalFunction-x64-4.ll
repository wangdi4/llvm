; RUN: opt -passes=dpcpp-kernel-barrier -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-barrier -S < %s | FileCheck %s
;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and the non-uniform value "%y"
;;           that crosses the barrier instruction and is an input to function "foo",
;;           which contains barrier itself and returns i32 value type that is crossing barrier!
;; The expected result:
;;      1. Kernel "main" contains no more barrier/dummy_barrier. instructions
;;      2. Kernel "main" stores "%y" value to offset 16 in the special buffer before calling "foo".
;;      3. Kernel "main" is still calling function "foo"
;;      4. Kernel "main" loads "%z" value from offset 24 in the special buffer after calling "foo".
;;      5. function "foo" contains no more barrier/dummy_barrier. instructions
;;      6. function "foo" stores "0" value to offset 24 in the special buffer before ret.
;;      6.1. This store of value "0" happens in a new created loop over all WIs
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
  %z = call i64 @foo(i64 %y)
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
;;;; TODO: add regular expression for the below values.
; CHECK: L2:
; CHECK: %SBIndex = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset = add nuw i64 %SBIndex, 16
; CHECK: %1 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset
; CHECK: %pSB_LocalId = bitcast i8* %1 to i64*
; CHECK: %SBIndex10 = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset11 = add nuw i64 %SBIndex10, 0
; CHECK: %2 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset11
; CHECK: %pSB_LocalId12 = bitcast i8* %2 to i64*
; CHECK: %loadedValue13 = load i64, i64* %pSB_LocalId12
; CHECK: store i64 %loadedValue13, i64* %pSB_LocalId
; CHECK: br label %CallBB
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: call i64 @foo
; CHECK: br label %
;;;; TODO: add regular expression for the below values.
; CHECK: SyncBB{{[0-9]*}}:                                          ; preds = %Dispatch{{[0-9]*}}, %L3
; CHECK: %SBIndex1 = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset2 = add nuw i64 %SBIndex1, 24
; CHECK: %11 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset2
; CHECK: %pSB_LocalId3 = bitcast i8* %11 to i64*
; CHECK: %loadedValue = load i64, i64* %pSB_LocalId3
; CHECK: %SBIndex4 = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset5 = add nuw i64 %SBIndex4, 8
; CHECK: %12 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset5
; CHECK: %pSB_LocalId6 = bitcast i8* %12 to i64*
; CHECK: store i64 %loadedValue, i64* %pSB_LocalId6
; CHECK: %SBIndex18 = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset19 = add nuw i64 %SBIndex18, 8
; CHECK: %13 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset19
; CHECK: %pSB_LocalId20 = bitcast i8* %13 to i64*
; CHECK: %loadedValue21 = load i64, i64* %pSB_LocalId20
; CHECK: %w = and i64 %loadedValue21, %loadedValue21
; CHECK: br label %L4
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: ret void
}

; CHECK: @foo
define i64 @foo(i64 %x) nounwind {
L1:
  call void @dummy_barrier.()
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 2)
  ret i64 0
; CHECK-NOT: @dummy_barrier.
; CHECK-NOT: @_Z18work_group_barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: LoopBB:
; CHECK: %SBIndex1 = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset = add nuw i64 %SBIndex1, 24
; CHECK: %7 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset
; CHECK: %pSB_LocalId = bitcast i8* %7 to i64*
; CHECK: store i64 0, i64* %pSB_LocalId
; CHECK: %LocalId_0 = load i64, i64* %pLocalId_0
; CHECK: %8 = add nuw i64 %LocalId_0, 1
; CHECK: store i64 %8, i64* %pLocalId_0
; CHECK: %9 = icmp ult i64 %8, %LocalSize_0
; CHECK: br i1 %9, label %Dispatch, label %LoopEnd_0
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: ret i64 0
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
; DEBUGIFY-NOT: WARNING
