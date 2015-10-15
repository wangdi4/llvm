; RUN: opt -B-Barrier -verify -S < %s | FileCheck %s
;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and the non-uniform value "%y"
;;           that crosses the barrier instruction and is an input to function "foo",
;;           which contains barrier itself and returns i32 value type that is crossing barrier!
;; The expected result:
;;      1. Kernel "main" contains no more barrier/dummybarrier instructions
;;      2. Kernel "main" stores "%y" value to offset 8 in the special buffer before calling "foo".
;;      3. Kernel "main" is still calling function "foo"
;;      4. Kernel "main" loads "%z" value from offset 12 in the special buffer after calling "foo".
;;      5. function "foo" contains no more barrier/dummybarrier instructions
;;      6. function "foo" stores "0" value to offset 12 in the special buffer before ret.
;;      6.1. This store of value "0" happens in a new created loop over all WIs
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %x) nounwind {
L1:
  call void @dummybarrier.()
  %lid = call i32 @_Z12get_local_idj(i32 0)
  %y = xor i32 %x, %lid
  br label %L2
L2:
  call void @_Z7barrierj(i32 1)
  %z = call i32 @foo(i32 %y)
  br label %L3
L3:
  call void @dummybarrier.()
  %w = and i32 %z, %z
  br label %L4
L4:
  call void @_Z7barrierj(i32 1)
  ret void
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
; CHECK: xor
;;;; TODO: add regular expression for the below values.

; CHECK: L2:                                               ; preds = %SyncBB3
; CHECK: %SBIndex = load i32* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset = add nuw i32 %SBIndex, 8
; CHECK: %1 = getelementptr inbounds i8* %pSB, i32 %SB_LocalId_Offset
; CHECK: %pSB_LocalId = bitcast i8* %1 to i32*
; CHECK: %SBIndex10 = load i32* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset11 = add nuw i32 %SBIndex10, 0
; CHECK: %2 = getelementptr inbounds i8* %pSB, i32 %SB_LocalId_Offset11
; CHECK: %pSB_LocalId12 = bitcast i8* %2 to i32*
; CHECK: %loadedValue13 = load i32* %pSB_LocalId12
; CHECK: store i32 %loadedValue13, i32* %pSB_LocalId
; CHECK: br label %CallBB
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: call i32 @foo
; CHECK: br label %
;;;; TODO: add regular expression for the below values.
; CHECK: SyncBB2:
; CHECK: %SBIndex1 = load i32* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset2 = add nuw i32 %SBIndex1, 12
; CHECK: %11 = getelementptr inbounds i8* %pSB, i32 %SB_LocalId_Offset2
; CHECK: %pSB_LocalId3 = bitcast i8* %11 to i32*
; CHECK: %loadedValue = load i32* %pSB_LocalId3
; CHECK: %SBIndex4 = load i32* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset5 = add nuw i32 %SBIndex4, 4
; CHECK: %12 = getelementptr inbounds i8* %pSB, i32 %SB_LocalId_Offset5
; CHECK: %pSB_LocalId6 = bitcast i8* %12 to i32*
; CHECK: store i32 %loadedValue, i32* %pSB_LocalId6
; CHECK: %SBIndex18 = load i32* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset19 = add nuw i32 %SBIndex18, 4
; CHECK: %13 = getelementptr inbounds i8* %pSB, i32 %SB_LocalId_Offset19
; CHECK: %pSB_LocalId20 = bitcast i8* %13 to i32*
; CHECK: %loadedValue21 = load i32* %pSB_LocalId20
; CHECK: %w = and i32 %loadedValue21, %loadedValue21
; CHECK: br label %L4
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
; CHECK: ret void
}

; CHECK: @foo
define i32 @foo(i32 %x) nounwind {
L1:
  call void @dummybarrier.()
  br label %L2
L2:
  call void @_Z7barrierj(i32 2)
  ret i32 0
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: LoopBB:
; CHECK: %SBIndex1 = load i32* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset = add nuw i32 %SBIndex1, 12
; CHECK: %7 = getelementptr inbounds i8* %pSB, i32 %SB_LocalId_Offset
; CHECK: %pSB_LocalId = bitcast i8* %7 to i32*
; CHECK: store i32 0, i32* %pSB_LocalId
; CHECK: %LocalId_0 = load i32* %pLocalId_0
; CHECK: %8 = add nuw i32 %LocalId_0, 1
; CHECK: store i32 %8, i32* %pLocalId_0
; CHECK: %9 = icmp ult i32 %8, %LocalSize_0
; CHECK: br i1 %9, label %Dispatch, label %LoopEnd_0
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: ret i32 0
}

declare void @_Z7barrierj(i32)
declare i32 @_Z12get_local_idj(i32)
declare void @dummybarrier.()

!opencl.kernels = !{!0}
!opencl.compiler.options = !{}

!0 = !{void (i32)* @main, !1, !1, !"", !"int", !"opencl_main_locals_anchor", !2, !3, !4, !5, !""}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{i32 0}
!3 = !{i32 3}
!4 = !{!"int"}
!5 = !{!"x"}
