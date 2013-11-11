; RUN: llvm-as %s -o %t.bc
; RUN: opt -B-Barrier -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and the non-uniform value "%y"
;;           that crosses the barrier instruction and is an input to function "foo",
;;           which contains barrier itself and returns void.
;;           kernel main also calls same "foo" function with uniform value "%x"
;; The expected result:
;;      1. Kernel "main" contains no more barrier/dummybarrier instructions
;;      2. Kernel "main" stores "%y" value to offset 8 in the special buffer before calling "foo".
;;      3. Kernel "main" is still calling function "foo"
;;      4. Kernel "main" stores "%x" value to offset 8 in the special buffer before calling "foo".
;;      5. Kernel "main" is still calling function "foo"
;;      6. function "foo" contains no more barrier/dummybarrier instructions
;;      7. function "foo" loads "%x" value from offset 8 in the special buffer before xor.
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
  call void @foo(i64 %y)
  br label %L2A
L2A:
  call void @dummybarrier.()
  br label %L3
L3:
  call void @_Z7barrierj(i32 1)
  call void @foo(i64 %x)
  br label %L3A
L3A:
  call void @dummybarrier.()
  ret void
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
; CHECK: xor
; CHECK: br label %
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: L2:
; CHECK:   %loadedCurrSB2 = load i64* %CurrSBIndex
; CHECK:   %"&(pSB[currWI].offset)3" = add nuw i64 %loadedCurrSB2, 8
; CHECK:   %"&pSB[currWI].offset4" = getelementptr inbounds i8* %pSB, i64 %"&(pSB[currWI].offset)3"
; CHECK:   %CastToValueType5 = bitcast i8* %"&pSB[currWI].offset4" to i64*
; CHECK:   %loadedCurrSB10 = load i64* %CurrSBIndex
; CHECK:   %"&(pSB[currWI].offset)11" = add nuw i64 %loadedCurrSB10, 0
; CHECK:   %"&pSB[currWI].offset12" = getelementptr inbounds i8* %pSB, i64 %"&(pSB[currWI].offset)11"
; CHECK:   %CastToValueType13 = bitcast i8* %"&pSB[currWI].offset12" to i64*
; CHECK:   %loadedValue = load i64* %CastToValueType13
; CHECK:   store i64 %loadedValue, i64* %CastToValueType5
; CHECK:   br label %SyncBB1
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: call void @foo
; CHECK: br label %
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: L3:
; CHECK:   %loadedCurrSB = load i64* %CurrSBIndex
; CHECK:   %"&(pSB[currWI].offset)" = add nuw i64 %loadedCurrSB, 8
; CHECK:   %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSB, i64 %"&(pSB[currWI].offset)"
; CHECK:   %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i64*
; CHECK:   store i64 %x, i64* %CastToValueType
; CHECK:   br label %SyncBB
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: call void @foo
; CHECK: br label %
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
; CHECK: ret
}

; CHECK: @foo
define void @foo(i64 %x) nounwind {
L1:
  call void @dummybarrier.()
  %y = xor i64 %x, %x
  br label %L2
L2:
  call void @_Z7barrierj(i32 2)
  ret void
; CHECK-NOT: @dummybarrier.
; CHECK-NOT: @_Z7barrierj
;;;; TODO: add regular expression for the below values.
; CHECK: SyncBB1:
; CHECK:   %loadedCurrSB = load i64* %CurrSBIndex
; CHECK:   %"&(pSB[currWI].offset)" = add nuw i64 %loadedCurrSB, 8
; CHECK:   %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSB, i64 %"&(pSB[currWI].offset)"
; CHECK:   %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i64*
; CHECK:   %loadedValue = load i64* %CastToValueType
; CHECK:   %y = xor i64 %loadedValue, %loadedValue
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
