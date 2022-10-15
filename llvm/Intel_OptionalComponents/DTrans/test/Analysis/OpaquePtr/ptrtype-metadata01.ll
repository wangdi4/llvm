; REQUIRES: asserts
; RUN: opt < %s -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt < %s -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt < %s -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt < %s -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that a getelementptr instruction tagged with
; the dtrans-type metadata extracts the type from the metadata when
; collecting the pointer types.

; Test case where metadata is placed on a GEP to indicate a desired type
; for use by the analysis.
%struct.test01a = type { i64, i64 }
%struct.test01b = type { %struct.test01a**, %struct.test01a** }
@glob_test01 = internal unnamed_addr global %struct.test01b zeroinitializer

define i32 @test01() {
  %ptr = call i8* @malloc(i64 160)

  ; Use metadata to indicate this result should be treated as a %struct.test01a**
  %ar1 = getelementptr i8, i8* %ptr, i32 0, !dtrans-type !0

  %ar1_st = bitcast i8* %ar1 to %struct.test01a**
  store %struct.test01a** %ar1_st, %struct.test01a*** getelementptr (%struct.test01b, %struct.test01b* @glob_test01, i32 0, i32 0)

  ; Use metadata to indicate this result should be treated as a %struct.test01a**
  %ar2 = getelementptr i8, i8* %ptr, i32 80, !dtrans-type !0
  %ar2_st = bitcast i8* %ar2 to %struct.test01a**
  store %struct.test01a** %ar2_st, %struct.test01a*** getelementptr (%struct.test01b, %struct.test01b* @glob_test01, i32 0, i32 1)

  ret i32 0
}
; CHECK-LABEL: define i32 @test01
; CHECK-NONOPAQUE: %ar1 = getelementptr i8, i8* %ptr, i32 0
; CHECK-OPAQUE: %ar1 = getelementptr i8, ptr %ptr, i32 0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     %struct.test01a**
; CHECK-NEXT:     i8*
; CHECK-NEXT:   No element pointees.

; CHECK-NONOPAQUE:  %ar2 = getelementptr i8, i8* %ptr, i32 80
; CHECK-OPAQUE: %ar2 = getelementptr i8, ptr %ptr, i32 80
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
  %ptr = call i8* @malloc(i64 160)

  ; Use metadata to indicate this result should be treated as a %struct.test02bad**
  %ar1 = getelementptr i8, i8* %ptr, i32 0, !dtrans-type !1
  %ar1_st = bitcast i8* %ar1 to %struct.test02a**
  store %struct.test02a** %ar1_st, %struct.test02a*** getelementptr (%struct.test02b, %struct.test02b* @glob_test02, i32 0, i32 0)

  ; Use metadata to indicate this result should be treated as a %struct.test02bad**
  %ar2 = getelementptr i8, i8* %ptr, i32 80, !dtrans-type !1
  %ar2_st = bitcast i8* %ar2 to %struct.test02a**
  store %struct.test02a** %ar2_st, %struct.test02a*** getelementptr (%struct.test02b, %struct.test02b* @glob_test02, i32 0, i32 1)

  ret i32 0
}
; CHECK-LABEL: define i32 @test02
; CHECK-NONOPAQUE: %ar1 = getelementptr i8, i8* %ptr, i32 0
; CHECK-OPAQUE: %ar1 = getelementptr i8, ptr %ptr, i32 0
; CHECK-NEXT: LocalPointerInfo: CompletelyAnalyzed
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     %struct.test02bad**
; CHECK-NEXT:     i8*
; CHECK-NEXT:   No element pointees.

; CHECK-NONOPAQUE: %ar2 = getelementptr i8, i8* %ptr, i32 80
; CHECK-OPAQUE: %ar2 = getelementptr i8, ptr %ptr, i32 80
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
