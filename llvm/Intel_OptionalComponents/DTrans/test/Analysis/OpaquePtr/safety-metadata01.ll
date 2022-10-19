; REQUIRES: asserts
; RUN: opt < %s -dtransop-allow-typed-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types 2>&1 | FileCheck %s
; RUN: opt < %s -dtransop-allow-typed-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types  2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types  2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that a getelementptr instruction tagged with metadata
; that explicitly marks the type results in the safety analyzer using that
; information when setting the safety flags.

; Test case where metadata is placed on a GEP to indicate a desired type
; for use by the analysis.
%struct.test01a = type { i64, i64 }
%struct.test01b = type { %struct.test01a**, %struct.test01a** }
@glob_test01 = internal unnamed_addr global %struct.test01b zeroinitializer

define dso_local i32 @test01() {
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
; CHECK-LABEL: LLVMType: %struct.test01a
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01a
; CHECK-LABEL: LLVMType: %struct.test01b
; CHECK: Safety data: Global instance{{ *}}
; CHECK: End LLVMType: %struct.test01b

; Test where an instruction is marked with metadata that is not the expected
; type results in safety flags being set on the types.
%struct.test02a = type { i64, i64 }
%struct.test02b = type { %struct.test02a**, %struct.test02a**}
%struct.test02bad = type { i64, i64 }
@glob_test02 = internal unnamed_addr global %struct.test02b zeroinitializer

define dso_local i32 @test02() {
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
; CHECK-LABEL: LLVMType: %struct.test02a
; CHECK: Safety data: Bad casting{{ *}}
; CHECK: End LLVMType: %struct.test02a
; CHECK-LABEL: LLVMType: %struct.test02b
; CHECK: Safety data: Bad casting | Mismatched element access | Global instance{{ *}}
; CHECK: End LLVMType: %struct.test02b
; CHECK-LABEL: LLVMType: %struct.test02bad
; CHECK: Safety data: Bad casting{{ *}}
; CHECK: End LLVMType: %struct.test02bad

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
