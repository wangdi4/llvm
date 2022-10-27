; REQUIRES: asserts
; RUN: opt -disable-output -debug-only=dtrans-alloc-collector -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that the user allocation and free functions from 557.xz are recognized
; when the IR is in the expected form following the DTrans eliminate read-only
; field access pass. (CMPLRLLVM-31111)

; CHECK-DAG: Identified as user free function: lzma_free
; CHECK-DAG: Identified as user allocation function: lzma_alloc

%struct.test = type { i64, i64 }

define internal "intel_dtrans_func_index"="1" i8* @lzma_alloc(i64 %arg) unnamed_addr !intel.dtrans.func.type !3 {
bb:
  %i = icmp eq i64 %arg, 0
  %i1 = select i1 %i, i64 1, i64 %arg
  br label %bb2

bb2:                                              ; preds = %bb
  %i3 = tail call noalias align 16 i8* @malloc(i64 %i1)
  br label %bb4

bb4:                                              ; preds = %bb2
  ret i8* %i3
}

define internal void @lzma_free(i8* "intel_dtrans_func_index"="1" %arg) unnamed_addr !intel.dtrans.func.type !4 {
bb:
  br label %bb1

bb1:                                              ; preds = %bb
  tail call void @free(i8* %arg)
  br label %bb2

bb2:                                              ; preds = %bb1
  ret void
}

define i64 @test() {
  %p = call i8* @lzma_alloc(i64 16)

  %ps = bitcast i8* %p to %struct.test*
  %f0 = getelementptr %struct.test, %struct.test* %ps, i64 0, i32 0
  %f1 = getelementptr %struct.test, %struct.test* %ps, i64 0, i32 1
  %v = load i64, i64* %f1
  store i64 0, i64* %f0
  store i64 1, i64* %f1

  %orig = bitcast %struct.test* %ps to i8*
  call void @lzma_free(i8* %orig)

  ret i64 %v
}

declare !intel.dtrans.func.type !5 void @free(i8* "intel_dtrans_func_index"="1") #1
declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!1 = !{i64 0, i32 0}  ; i64
!2 = !{i8 0, i32 1}  ; i8*
!3 = distinct !{!2}
!4 = distinct !{!2}
!5 = distinct !{!2}
!6 = distinct !{!2}
!7 = !{!"S", %struct.test zeroinitializer, i32 2, !1, !1} ; { i64, i64 }

!intel.dtrans.types = !{!7}
