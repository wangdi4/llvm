; RUN: opt < %s -S -whole-program-assume -dtrans-optbasetest -dtrans-optbasetest-typelist=struct.test01a | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-optbasetest -dtrans-optbasetest-typelist=struct.test01a  | FileCheck %s

; This test verifies that type information used to inform DTrans analysis
; about types gets remapped when the type names is changed by a DTrans
; transformation.

%struct.test01a = type { i64, i64 }
%struct.test01b = type { %struct.test01a**, %struct.test01a**}
@glob_test01 = internal unnamed_addr global %struct.test01b zeroinitializer

define dso_local i32 @test01() {
  %ptr = call i8* @malloc(i64 160)

  ; Use metadata to indicate this result should be treated as a %struct.test01a**
  %ar1 = getelementptr i8, i8* %ptr, i32 0, !dtrans-type !0
; CHECK:   %ar1 = getelementptr i8, i8* %ptr, i32 0, !dtrans-type !0

  %ar1_st = bitcast i8* %ar1 to %struct.test01a**
  store %struct.test01a** %ar1_st, %struct.test01a*** getelementptr (%struct.test01b, %struct.test01b* @glob_test01, i32 0, i32 0)

  ; Use metadata to indicate this result should be treated as a %struct.test01a**
  %ar2 = getelementptr i8, i8* %ptr, i32 80, !dtrans-type !0
; CHECK:  %ar2 = getelementptr i8, i8* %ptr, i32 80, !dtrans-type !0

  %ar2_st = bitcast i8* %ar2 to %struct.test01a**
  store %struct.test01a** %ar2_st, %struct.test01a*** getelementptr (%struct.test01b, %struct.test01b* @glob_test01, i32 0, i32 1)

  ret i32 0
}
!0 = !{ %struct.test01a** null}
; CHECK: !0 = !{%__DTT_struct.test01a** null}

declare i8* @malloc(i64)
