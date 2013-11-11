; RUN: llvm-as %s -o %t.bc
; RUN: opt -B-Barrier -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and the non-uniform value "%y"
;;           that crosses the barrier instruction and is an input to function "foo",
;;           which contains barrier itself and returns i32 value type that is crossing barrier!
;; The expected result:
;;      1. Kernel "main" contains no more barrier/dummybarrier instructions
;;      2. Kernel "main" stores "%y" value to offset 16 in the special buffer before calling "foo".
;;      3. Kernel "main" is still calling function "foo"
;;      4. Kernel "main" loads "%z" value from offset 24 in the special buffer after calling "foo".
;;      5. function "foo" contains no more barrier/dummybarrier instructions
;;      6. function "foo" stores "0" value to offset 24 in the special buffer before ret.
;;      6.1. This store of value "0" happens in a new created loop over all WIs
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
  %z = call i64 @foo(i64 %y)
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
;;;; TODO: add regular expression for the below values.
; CHECK: L2:
; CHECK:   %loadedCurrSB = load i64* %CurrSBIndex
; CHECK:   %"&(pSB[currWI].offset)" = add nuw i64 %loadedCurrSB, 16
; CHECK:   %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSB, i64 %"&(pSB[currWI].offset)"
; CHECK:   %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i64*
; CHECK:   %loadedCurrSB13 = load i64* %CurrSBIndex
; CHECK:   %"&(pSB[currWI].offset)14" = add nuw i64 %loadedCurrSB13, 0
; CHECK:   %"&pSB[currWI].offset15" = getelementptr inbounds i8* %pSB, i64 %"&(pSB[currWI].offset)14"
; CHECK:   %CastToValueType16 = bitcast i8* %"&pSB[currWI].offset15" to i64*
; CHECK:   %loadedValue17 = load i64* %CastToValueType16
; CHECK:   store i64 %loadedValue17, i64* %CastToValueType
; CHECK:   br label %SyncBB
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: call i64 @foo
; CHECK: br label %
;;;; TODO: add regular expression for the below values.
; CHECK: SyncBB30:
; CHECK:   %loadedCurrSB1 = load i64* %CurrSBIndex
; CHECK:   %"&(pSB[currWI].offset)2" = add nuw i64 %loadedCurrSB1, 24
; CHECK:   %"&pSB[currWI].offset3" = getelementptr inbounds i8* %pSB, i64 %"&(pSB[currWI].offset)2"
; CHECK:   %CastToValueType4 = bitcast i8* %"&pSB[currWI].offset3" to i64*
; CHECK:   %loadedValue = load i64* %CastToValueType4
; CHECK:   %loadedCurrSB5 = load i64* %CurrSBIndex
; CHECK:   %"&(pSB[currWI].offset)6" = add nuw i64 %loadedCurrSB5, 8
; CHECK:   %"&pSB[currWI].offset7" = getelementptr inbounds i8* %pSB, i64 %"&(pSB[currWI].offset)6"
; CHECK:   %CastToValueType8 = bitcast i8* %"&pSB[currWI].offset7" to i64*
; CHECK:   store i64 %loadedValue, i64* %CastToValueType8
; CHECK:   %loadedCurrSB23 = load i64* %CurrSBIndex
; CHECK:   %"&(pSB[currWI].offset)24" = add nuw i64 %loadedCurrSB23, 8
; CHECK:   %"&pSB[currWI].offset25" = getelementptr inbounds i8* %pSB, i64 %"&(pSB[currWI].offset)24"
; CHECK:   %CastToValueType26 = bitcast i8* %"&pSB[currWI].offset25" to i64*
; CHECK:   %loadedValue27 = load i64* %CastToValueType26
; CHECK:   %w = and i64 %loadedValue27, %loadedValue27
; CHECK:   br label %L4
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
; CHECK: ret void
}

; CHECK: @foo
define i64 @foo(i64 %x) nounwind {
L1:
  call void @dummybarrier.()
  br label %L2
L2:
  call void @_Z7barrierj(i32 2)
  ret i64 0
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: loopBB:
; CHECK:   %loadedCurrSB1 = load i64* %CurrSBIndex
; CHECK:   %"&(pSB[currWI].offset)" = add nuw i64 %loadedCurrSB1, 24
; CHECK:   %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSB, i64 %"&(pSB[currWI].offset)"
; CHECK:   %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i64*
; CHECK:   store i64 0, i64* %CastToValueType
; CHECK:   %loadedCurrWI = load i64* %CurrWI
; CHECK:   %check.WI.iter = icmp ult i64 %loadedCurrWI, %IterCount
; CHECK:   %"CurrWI++" = add nuw i64 %loadedCurrWI, 1
; CHECK:   store i64 %"CurrWI++", i64* %CurrWI
; CHECK:   %loadedCurrSB = load i64* %CurrSBIndex
; CHECK:   %"loadedCurrSB+Stride" = add nuw i64 %loadedCurrSB, 32
; CHECK:   store i64 %"loadedCurrSB+Stride", i64* %CurrSBIndex
; CHECK:   br i1 %check.WI.iter, label %loopBB, label %RetBB
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: ret i64 0
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
