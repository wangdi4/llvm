; RUN: opt -B-Barrier -verify -S < %s | FileCheck %s

;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and the non-uniform value "%y"
;;           that crosses the barrier instruction and is an input to function "foo",
;;           which contains barrier itself and returns void.
;; The expected result:
;;      1. Kernel "main" contains no more barrier/dummybarrier instructions
;;      2. Kernel "main" stores "%y" value to offset 4 in the special buffer before calling "foo".
;;      3. Kernel "main" is still calling function "foo"
;;      4. function "foo" contains no more barrier/dummybarrier instructions
;;      5. function "foo" loads "%x" value from offset 4 in the special buffer before xor.
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
  call void @foo(i32 %y)
  br label %L3
L3:
  call void @dummybarrier.()
  ret void
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
; CHECK: xor
;;;; TODO: add regular expression for the below values.
; CHECK: L2:
; CHECK:   %SBIndex = load i32* %pCurrSBIndex
; CHECK:   %SB_LocalId_Offset = add nuw i32 %SBIndex, 4
; CHECK:   [[GEP0:%[a-zA-Z0-9]+]] = getelementptr inbounds i8* %pSB, i32 %SB_LocalId_Offset
; CHECK:   %pSB_LocalId = bitcast i8* [[GEP0]] to i32*
; CHECK:   [[SBIndex1:%SBIndex[a-zA-Z0-9]+]] = load i32* %pCurrSBIndex
; CHECK:   [[SB_LocalId_Offset1:%SB_LocalId_Offset[a-zA-Z0-9]+]] = add nuw i32 [[SBIndex1]], 0
; CHECK:   [[GEP1:%[a-zA-Z0-9]+]] = getelementptr inbounds i8* %pSB, i32 [[SB_LocalId_Offset1]]
; CHECK:   [[pSB_LocalId1:%pSB_LocalId[a-zA-Z0-9]+]] = bitcast i8* [[GEP1]] to i32*
; CHECK:   %loadedValue = load i32* [[pSB_LocalId1]]
; CHECK:   store i32 %loadedValue, i32* %pSB_LocalId
; CHECK:   br label %CallBB
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: call void @foo
; CHECK: br label %
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
; CHECK: ret
}

; CHECK: @foo
define void @foo(i32 %x) nounwind {
L1:
  call void @dummybarrier.()
  %y = xor i32 %x, %x
  br label %L2
L2:
  call void @_Z7barrierj(i32 2)
  ret void
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: SyncBB1:
; CHECK:   %SBIndex = load i32* %pCurrSBIndex
; CHECK:   %SB_LocalId_Offset = add nuw i32 %SBIndex, 4
; CHECK:   [[GEP0:%[a-zA-Z0-9]+]] = getelementptr inbounds i8* %pSB, i32 %SB_LocalId_Offset
; CHECK:   %pSB_LocalId = bitcast i8* [[GEP0]] to i32*
; CHECK:   %loadedValue = load i32* %pSB_LocalId
; CHECK:   %y = xor i32 %loadedValue, %loadedValue
; CHECK:   br label %L2
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
; CHECK: ret
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
