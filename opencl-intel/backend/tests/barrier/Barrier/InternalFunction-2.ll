; RUN: llvm-as %s -o %t.bc
; RUN: opt -B-Barrier -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and the non-uniform value "%y"
;;           that crosses the barrier instruction and is an input to function "foo",
;;           which contains barrier itself and returns i32 value type that is not crossing barrier!
;; The expected result:
;;      1. Kernel "main" contains no more barrier/dummybarrier instructions
;;      2. Kernel "main" stores "%y" value to offset 4 in the special buffer before calling "foo".
;;      3. Kernel "main" is still calling function "foo"
;;      4. function "foo" contains no more barrier/dummybarrier instructions
;;      5. function "foo" loads "%x" value from offset 4 in the special buffer before xor.
;;      6. function "foo" does not store ret value ("%x") in the special buffer before ret.
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
  ret void
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
; CHECK: xor
;;;; TODO: add regular expression for the below values.
; CHECK: L2:
; CHECK:   %loadedCurrSB = load i32* %CurrSBIndex
; CHECK:   %"&(pSB[currWI].offset)" = add nuw i32 %loadedCurrSB, 4
; CHECK:   %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSB, i32 %"&(pSB[currWI].offset)"
; CHECK:   %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i32*
; CHECK:   %loadedCurrSB5 = load i32* %CurrSBIndex
; CHECK:   %"&(pSB[currWI].offset)6" = add nuw i32 %loadedCurrSB5, 0
; CHECK:   %"&pSB[currWI].offset7" = getelementptr inbounds i8* %pSB, i32 %"&(pSB[currWI].offset)6"
; CHECK:   %CastToValueType8 = bitcast i8* %"&pSB[currWI].offset7" to i32*
; CHECK:   %loadedValue = load i32* %CastToValueType8
; CHECK:   store i32 %loadedValue, i32* %CastToValueType
; CHECK:   br label %SyncBB
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: call i32 @foo
; CHECK: br label %
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
; CHECK: ret
}

; CHECK: @foo
define i32 @foo(i32 %x) nounwind {
L1:
  call void @dummybarrier.()
  %y = xor i32 %x, %x
  br label %L2
L2:
  call void @_Z7barrierj(i32 2)
  ret i32 %x
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: SyncBB6:
; CHECK:   %loadedCurrSB1 = load i32* %CurrSBIndex
; CHECK:   %"&(pSB[currWI].offset)2" = add nuw i32 %loadedCurrSB1, 4
; CHECK:   %"&pSB[currWI].offset3" = getelementptr inbounds i8* %pSB, i32 %"&(pSB[currWI].offset)2"
; CHECK:   %CastToValueType4 = bitcast i8* %"&pSB[currWI].offset3" to i32*
; CHECK:   %loadedValue5 = load i32* %CastToValueType4
; CHECK:   %y = xor i32 %loadedValue5, %loadedValue5
; CHECK:   br label %L2
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: SyncBB:
; CHECK:   %loadedCurrSB = load i32* %CurrSBIndex
; CHECK:   %"&(pSB[currWI].offset)" = add nuw i32 %loadedCurrSB, 4
; CHECK:   %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSB, i32 %"&(pSB[currWI].offset)"
; CHECK:   %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i32*
; CHECK:   %loadedValue = load i32* %CastToValueType
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
; CHECK: ret i32 %loadedValue
}

declare void @_Z7barrierj(i32)
declare i32 @_Z12get_local_idj(i32)
declare void @dummybarrier.()

!opencl.kernels = !{!0}
!opencl.build.options = !{}

!0 = metadata !{void (i32)* @main, metadata !1, metadata !1, metadata !"", metadata !"int", metadata !"opencl_main_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !""}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 0}
!3 = metadata !{i32 3}
!4 = metadata !{metadata !"int"}
!5 = metadata !{metadata !"x"}
