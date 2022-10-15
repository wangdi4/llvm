; REQUIRES: asserts
; This test verifies that frequency heuristic is used to select candidate
; for DynClone transformation.

;  RUN: opt < %s -dtransop-allow-typed-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -dtrans-dyncloneop -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s
;  RUN: opt < %s -dtransop-allow-typed-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-dyncloneop -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; All %struct.test.01, %struct.test.02 and %struct.test.03 have same
; exact layout. None of them violate any safety issues.
; But, only fields in %struct.test.01 are selected as candidates since
; %struct.test.01 has highest total frequency.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i64 }
%struct.test.02 = type { i32, i64, i32, i32, i16, i64*, i64, i64 }
%struct.test.03 = type { i32, i64, i32, i32, i16, i64*, i64, i64 }

; CHECK-DAG:     Rejecting struct.test.03 based on heuristic.
; CHECK-DAG:     Rejecting struct.test.02 based on heuristic.
; CHECK-DAG:     Possible Candidate fields:
; CHECK-DAG:     struct: struct.test.01 Index: 1
; CHECK-DAG:     struct: struct.test.01 Index: 6
; CHECK-DAG:     struct: struct.test.01 Index: 7

define void @init() {
  %call1 = tail call i8* @calloc(i64 10, i64 56)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  store i64 %g1, i64* %F6, align 8
  %F7 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 7
  store i64 %g1, i64* %F7, align 8
  ret void
}

define void @proc1() {
  %call3 = tail call i8* @calloc(i64 1, i64 56)
  %tp2 = bitcast i8* %call3 to %struct.test.01*
  %call4 = tail call i8* @calloc(i64 1, i64 56)
  %tp4 = bitcast i8* %call4 to %struct.test.02*
  %F7 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 7
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  store i64 1, i64* %F6, align 8
  %F1_2 = getelementptr %struct.test.02, %struct.test.02* %tp4, i32 0, i32 1
  store i64 2, i64* %F7, align 8
  ret void
}

define void @proc2() {
  %call2 = tail call i8* @calloc(i64 1, i64 56)
  %tp3 = bitcast i8* %call2 to %struct.test.01*
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp3, i32 0, i32 1
  %F7 = getelementptr %struct.test.01, %struct.test.01* %tp3, i32 0, i32 7
  %ld3 = load i64, i64* %F7
  store i64 %ld3, i64* %F1, align 8
  %call5 = tail call i8* @calloc(i64 1, i64 56)
  %tp5 = bitcast i8* %call5 to %struct.test.03*
  %F6 = getelementptr %struct.test.03, %struct.test.03* %tp5, i32 0, i32 6
  store i64 100, i64* %F6, align 8
  ret void
}

define i32 @main() {
entry:
  call void @init();
  call void @proc1();
  call void @proc2();
  ret i32 0
}

declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5}
!7 = !{!"S", %struct.test.01 zeroinitializer, i32 8, !1, !2, !1, !1, !3, !4, !2, !2} ; { i32, i64, i32, i32, i16, i64*, i64, i64 }
!8 = !{!"S", %struct.test.02 zeroinitializer, i32 8, !1, !2, !1, !1, !3, !4, !2, !2} ; { i32, i64, i32, i32, i16, i64*, i64, i64 }
!9 = !{!"S", %struct.test.03 zeroinitializer, i32 8, !1, !2, !1, !1, !3, !4, !2, !2} ; { i32, i64, i32, i32, i16, i64*, i64, i64 }

!intel.dtrans.types = !{!7, !8, !9}
