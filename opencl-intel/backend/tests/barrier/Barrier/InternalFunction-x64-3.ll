; RUN: opt -B-Barrier -verify -S < %s | FileCheck %s
;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and function "foo",
;;           which contains barrier itself and returns i32 value type,
;;           and receives uniform value "%x" that does not cross barrier.
;; The expected result:
;;      1. Kernel "main" contains no more barrier/dummybarrier instructions
;;      2. Kernel "main" is still calling function "foo"
;;      3. Kernel "main" loads "%z" value from offset 8 in the special buffer after calling "foo".
;;      4. function "foo" contains no more barrier/dummybarrier instructions
;;      5. function "foo" stores "%x" value to offset 8 in the special buffer before ret.
;;      5.1. This store of value "%x" happens in a new created loop over all WIs
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"

target triple = "x86_64-pc-win32"
; CHECK: @main
define void @main(i64 %x) nounwind {
L1:
  call void @dummybarrier.()
  %lid = call i64 @_Z12get_local_idj(i32 0)
  %y = xor i64 %x, %lid
  br label %L2
L2:
  call void @_Z7barrierj(i32 1)
  %z = call i64 @foo(i64 %x)
  br label %L3
L3:
  call void @dummybarrier.()
  %w = and i64 %z, %z
  br label %L4
L4:
  call void @_Z7barrierj(i32 1)
  ret void
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
; CHECK: xor
; CHECK: call i64 @foo
; CHECK: br label %
;;;; TODO: add regular expression for the below values.

; CHECK: SyncBB2:                                          ; preds = %Dispatch, %L3
; CHECK: %SBIndex = load i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset = add nuw i64 %SBIndex, 8
; CHECK: %7 = getelementptr inbounds i8* %pSB, i64 %SB_LocalId_Offset
; CHECK: %pSB_LocalId = bitcast i8* %7 to i64*
; CHECK: %loadedValue = load i64* %pSB_LocalId
; CHECK: %SBIndex1 = load i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset2 = add nuw i64 %SBIndex1, 0
; CHECK: %8 = getelementptr inbounds i8* %pSB, i64 %SB_LocalId_Offset2
; CHECK: %pSB_LocalId3 = bitcast i8* %8 to i64*
; CHECK: store i64 %loadedValue, i64* %pSB_LocalId3
; CHECK: %SBIndex4 = load i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset5 = add nuw i64 %SBIndex4, 0
; CHECK: %9 = getelementptr inbounds i8* %pSB, i64 %SB_LocalId_Offset5
; CHECK: %pSB_LocalId6 = bitcast i8* %9 to i64*
; CHECK: %loadedValue7 = load i64* %pSB_LocalId6
; CHECK: %w = and i64 %loadedValue7, %loadedValue7
; CHECK: br label %L4
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
; CHECK: ret
}

; CHECK: @foo
define i64 @foo(i64 %x) nounwind {
L1:
  call void @dummybarrier.()
  br label %L2
L2:
  call void @_Z7barrierj(i32 2)
  ret i64 %x
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: LoopBB:
; CHECK: %SBIndex1 = load i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset = add nuw i64 %SBIndex1, 8
; CHECK: %7 = getelementptr inbounds i8* %pSB, i64 %SB_LocalId_Offset
; CHECK: %pSB_LocalId = bitcast i8* %7 to i64*
; CHECK: store i64 %x, i64* %pSB_LocalId
; CHECK: %LocalId_0 = load i64* %pLocalId_0
; CHECK: %8 = add nuw i64 %LocalId_0, 1
; CHECK: store i64 %8, i64* %pLocalId_0
; CHECK: %9 = icmp ult i64 %8, %LocalSize_0
; CHECK: br i1 %9, label %Dispatch, label %LoopEnd_0
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: ret i64 %x
}

declare void @_Z7barrierj(i32)
declare i64 @_Z12get_local_idj(i32)
declare void @dummybarrier.()

!opencl.kernels = !{!0}
!opencl.compiler.options = !{}

!0 = !{void (i64)* @main, !1, !1, !"", !"int", !"opencl_main_locals_anchor", !2, !3, !4, !5, !""}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{i32 0}
!3 = !{i32 3}
!4 = !{!"int"}
!5 = !{!"x"}
