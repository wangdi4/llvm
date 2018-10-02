; RUN: opt < %s -disable-output -whole-program-assume -dtransanalysis -dtrans-print-types | FileCheck %s
; RUN: opt < %s -disable-output -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types | FileCheck %s

; This test verifies that a store instruction tagged with metadata
; that explicitly marks the type to avoid an unsafe pointer store.

; Test case where metadata is placed on a type to indicate a desired type
; for use by the analysis.
%struct.test01a = type { i64, i64 }
%struct.test01b = type { %struct.test01a**, %struct.test01a**}
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
!0 = !{ %struct.test01a** null}

; CHECK-LABEL: LLVMType: %struct.test01a = type { i64, i64 }
; CHECK: Safety data: No issues found
; CHECK-LABEL: LLVMType: %struct.test01b = type { %struct.test01a**, %struct.test01a** }
; CHECK: Safety data: Global instance


; Test where an instruction is marked with metadata that is not the right
; type. This should result in the safety bits being set when the local pointer
; analyzer sees additional types being associated.
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
!1 = !{ %struct.test02bad** null}

; CHECK-LABEL: LLVMType: %struct.test02a = type { i64, i64 }
; CHECK: Safety data: Bad casting | Unsafe pointer store
; CHECK-LABEL: LLVMType: %struct.test02b = type { %struct.test02a**, %struct.test02a** }
; CHECK: Safety data: Unsafe pointer store | Global instance
; CHECK-LABEL: LLVMType: %struct.test02bad = type { i64, i64 }
; CHECK: Safety data: Bad casting | Unsafe pointer store


declare i8* @malloc(i64)


