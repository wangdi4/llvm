; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; TODO: Remove the -opaque-pointers option. It is currently needed
; because global variables are not recognized as being opaque pointers yet.

; RUN: opt < %s -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results 2>&1 | FileCheck %s

; This test verifies that a getelementptr instruction tagged with
; the dtrans-type metadata extracts the type from the metadata when
; collecting the pointer types.

; This is version of the test ptrtype-metadata01.ll that directly uses
; opaque pointers instead of bitcast instructions.

; Test case where metadata is placed on a GEP to indicate a desired type
; for use by the analysis.
%struct.test01a = type { i64, i64 }
%struct.test01b = type { ptr, ptr }
@glob_test01 = internal unnamed_addr global %struct.test01b zeroinitializer

define i32 @test01() {
  %ptr = call ptr @malloc(i64 160)

  ; Use metadata to indicate this result should be treated as a %struct.test01a**
  %ar1_st = getelementptr i8, ptr %ptr, i32 0, !dtrans-type !0
  store ptr %ar1_st, ptr getelementptr (%struct.test01b, ptr @glob_test01, i32 0, i32 0)

  ; Use metadata to indicate this result should be treated as a %struct.test01a**
  %ar2_st = getelementptr i8, ptr %ptr, i32 80, !dtrans-type !0
  store ptr %ar2_st, ptr getelementptr (%struct.test01b, ptr @glob_test01, i32 0, i32 1)

  ret i32 0
}
; CHECK-LABEL: define i32 @test01
; CHECK: %ar1_st = getelementptr i8, ptr %ptr, i32 0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     %struct.test01a**
; CHECK-NEXT:     i8*
; CHECK-NEXT:   No element pointees.

; CHECK: %ar2_st = getelementptr i8, ptr %ptr, i32 80
; CHECK-NEXT: LocalPointerInfo: CompletelyAnalyzed
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     %struct.test01a**
; CHECK-NEXT:     i8*
; CHECK-NEXT:   No element pointees.

; Test where an instruction is marked with metadata that is different from
; what the pointer gets used as. When the safety analyzer is run on this
; it should produce a safety flag.
%struct.test02a = type { i64, i64 }
%struct.test02b = type { %struct.test02a**, %struct.test02a**}
%struct.test02bad = type { i64, i64 }
@glob_test02 = internal unnamed_addr global %struct.test02b zeroinitializer

define i32 @test02() {
  %ptr = call ptr @malloc(i64 160)

  ; Use metadata to indicate this result should be treated as a %struct.test02bad**
  %ar1_st = getelementptr i8, ptr %ptr, i32 0, !dtrans-type !1
  store ptr %ar1_st, ptr getelementptr (%struct.test02b, ptr @glob_test02, i32 0, i32 0)

  ; Use metadata to indicate this result should be treated as a %struct.test02bad**
  %ar2_st = getelementptr i8, ptr %ptr, i32 80, !dtrans-type !1
  store ptr %ar2_st, ptr getelementptr (%struct.test02b, ptr @glob_test02, i32 0, i32 1)

  ret i32 0
}
; CHECK-LABEL: define i32 @test02
; CHECK: %ar1_st = getelementptr i8, ptr %ptr, i32 0
; CHECK-NEXT: LocalPointerInfo: CompletelyAnalyzed
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     %struct.test02bad**
; CHECK-NEXT:     i8*
; CHECK-NEXT:   No element pointees.

; CHECK: %ar2_st = getelementptr i8, ptr %ptr, i32 80
; CHECK-NEXT: LocalPointerInfo: CompletelyAnalyzed
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     %struct.test02bad**
; CHECK-NEXT:     i8*
; CHECK-NEXT:   No element pointees.

declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" i8* @malloc(i64)

!0 = !{ %struct.test01a zeroinitializer, i32 2}
!1 = !{%struct.test02bad zeroinitializer, i32 2}
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.test01a zeroinitializer, i32 2}  ; %struct.test01a**
!4 = !{%struct.test02a zeroinitializer, i32 2}  ; %struct.test02a**
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5}
!7 = !{!"S", %struct.test01a zeroinitializer, i32 2, !2, !2} ; { i64, i64 }
!8 = !{!"S", %struct.test01b zeroinitializer, i32 2, !3, !3} ; { %struct.test01a**, %struct.test01a**}
!9 = !{!"S", %struct.test02a zeroinitializer, i32 2, !2, !2} ; { i64, i64 }
!10 = !{!"S", %struct.test02b zeroinitializer, i32 2, !4, !4} ; { %struct.test02a**, %struct.test02a**}
!11 = !{!"S", %struct.test02bad zeroinitializer, i32 2, !2, !2} ; { i64, i64 }

!intel.dtrans.types = !{!7, !8, !9, !10, !11}
