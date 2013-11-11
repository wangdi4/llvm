; RUN: llvm-as %s -o %t.bc
; RUN: opt -B-Barrier -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and the non-uniform value "%y"
;;           that crosses the barrier instruction and is an input to function "foo",
;;           which contains barrier itself and returns i64 value.
;;           kernel main also calls same "foo" function with non-uniform returned value value "%r1"
;; The expected result:
;;      1. Kernel "main" contains no more barrier/dummybarrier instructions
;;      2. Kernel "main" stores "%y" value to offset 16 in the special buffer before calling "foo".
;;      3. Kernel "main" is still calling function "foo"
;;      4. Kernel "main" loads "%r1" value from offset 24 in the special buffer after calling "foo".
;;      5. Kernel "main" stores "%r1" value to offset 16 in the special buffer before calling "foo".
;;      6. Kernel "main" is still calling function "foo"
;;      7. function "foo" contains no more barrier/dummybarrier instructions
;;      8. function "foo" loads "%x" value from offset 16 in the special buffer before xor.
;;      9. function "foo" stores "%y" value to offset 24 in the special buffer before ret.
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
  %r1 = call i64 @foo(i64 %y)
  br label %L2A
L2A:
  call void @dummybarrier.()
  br label %L3
L3:
  call void @_Z7barrierj(i32 1)
  %r2 = call i64 @foo(i64 %r1)
  br label %L3B
L3B:
  call void @dummybarrier.()
  ret void
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: L2:
; CHECK:   %loadedCurrSB2 = load i64* %CurrSBIndex
; CHECK:   %"&(pSB[currWI].offset)3" = add nuw i64 %loadedCurrSB2, 16
; CHECK:   %"&pSB[currWI].offset4" = getelementptr inbounds i8* %pSB, i64 %"&(pSB[currWI].offset)3"
; CHECK:   %CastToValueType5 = bitcast i8* %"&pSB[currWI].offset4" to i64*
; CHECK:   %loadedCurrSB18 = load i64* %CurrSBIndex
; CHECK:   %"&(pSB[currWI].offset)19" = add nuw i64 %loadedCurrSB18, 0
; CHECK:   %"&pSB[currWI].offset20" = getelementptr inbounds i8* %pSB, i64 %"&(pSB[currWI].offset)19"
; CHECK:   %CastToValueType21 = bitcast i8* %"&pSB[currWI].offset20" to i64*
; CHECK:   %loadedValue22 = load i64* %CastToValueType21
; CHECK:   store i64 %loadedValue22, i64* %CastToValueType5
; CHECK:   br label %SyncBB1
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
; CHECK: call i64 @foo
; CHECK: br label %
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: SyncBB41:
; CHECK:   %loadedCurrSB6 = load i64* %CurrSBIndex
; CHECK:   %"&(pSB[currWI].offset)7" = add nuw i64 %loadedCurrSB6, 24
; CHECK:   %"&pSB[currWI].offset8" = getelementptr inbounds i8* %pSB, i64 %"&(pSB[currWI].offset)7"
; CHECK:   %CastToValueType9 = bitcast i8* %"&pSB[currWI].offset8" to i64*
; CHECK:   %loadedValue = load i64* %CastToValueType9
; CHECK:   %loadedCurrSB10 = load i64* %CurrSBIndex
; CHECK:   %"&(pSB[currWI].offset)11" = add nuw i64 %loadedCurrSB10, 8
; CHECK:   %"&pSB[currWI].offset12" = getelementptr inbounds i8* %pSB, i64 %"&(pSB[currWI].offset)11"
; CHECK:   %CastToValueType13 = bitcast i8* %"&pSB[currWI].offset12" to i64*
; CHECK:   store i64 %loadedValue, i64* %CastToValueType13
; CHECK:   br label %L3
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; TODO: add regular expression for the below values.
; CHECK: L3:
; CHECK:   %loadedCurrSB = load i64* %CurrSBIndex
; CHECK:   %"&(pSB[currWI].offset)" = add nuw i64 %loadedCurrSB, 16
; CHECK:   %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSB, i64 %"&(pSB[currWI].offset)"
; CHECK:   %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i64*
; CHECK:   %loadedCurrSB28 = load i64* %CurrSBIndex
; CHECK:   %"&(pSB[currWI].offset)29" = add nuw i64 %loadedCurrSB28, 8
; CHECK:   %"&pSB[currWI].offset30" = getelementptr inbounds i8* %pSB, i64 %"&(pSB[currWI].offset)29"
; CHECK:   %CastToValueType31 = bitcast i8* %"&pSB[currWI].offset30" to i64*
; CHECK:   %loadedValue32 = load i64* %CastToValueType31
; CHECK:   store i64 %loadedValue32, i64* %CastToValueType
; CHECK:   br label %SyncBB
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: call i64 @foo
; CHECK: br label %
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
; CHECK: ret
}

; CHECK: @foo
define i64 @foo(i64 %x) nounwind {
L1:
  call void @dummybarrier.()
  %y = xor i64 %x, %x
  br label %L2
L2:
  call void @_Z7barrierj(i32 2)
  ret i64 %y
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: SyncBB5:
; CHECK:   %loadedCurrSB = load i64* %CurrSBIndex
; CHECK:   %"&(pSB[currWI].offset)" = add nuw i64 %loadedCurrSB, 16
; CHECK:   %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSB, i64 %"&(pSB[currWI].offset)"
; CHECK:   %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i64*
; CHECK:   %loadedValue = load i64* %CastToValueType
; CHECK:   %y = xor i64 %loadedValue, %loadedValue
; CHECK:   %loadedCurrSB1 = load i64* %CurrSBIndex
; CHECK:   %"&(pSB[currWI].offset)2" = add nuw i64 %loadedCurrSB1, 24
; CHECK:   %"&pSB[currWI].offset3" = getelementptr inbounds i8* %pSB, i64 %"&(pSB[currWI].offset)2"
; CHECK:   %CastToValueType4 = bitcast i8* %"&pSB[currWI].offset3" to i64*
; CHECK:   store i64 %y, i64* %CastToValueType4
; CHECK:   br label %L2
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
; CHECK: ret
}

declare void @_Z7barrierj(i32)
declare i64 @_Z12get_local_idj(i32)
declare void @dummybarrier.()

!opencl.kernels = !{!0}
!opencl.build.options = !{}

!0 = metadata !{void (i64)* @main, metadata !1, metadata !1, metadata !"", metadata !"int", metadata !"opencl_main_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !""}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 0}
!3 = metadata !{i32 3}
!4 = metadata !{metadata !"int"}
!5 = metadata !{metadata !"x"}
