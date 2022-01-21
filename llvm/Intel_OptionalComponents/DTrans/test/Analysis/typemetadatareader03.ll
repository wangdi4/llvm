; REQUIRES: asserts

; RUN: opt -dtrans-typemetadatareader -dtrans-typemetadatareader-missing -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes=dtrans-typemetadatareader -dtrans-typemetadatareader-missing -disable-output < %s 2>&1 | FileCheck %s

; This test is to check that definitions and instructions that are expected to
; have dtrans type metadata associated with them are detected when they are
; missing the metadata.

%struct.test01 = type { i64, i32, i32 }

; Check a global definition
@test01_var = internal global %struct.test01* null
; CHECK: Missing var type metadata: @test01_var = internal global %struct.test01* null

; Check a function definition
define internal %struct.test01* @test01() {
  %mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %mem to %struct.test01*
  ret %struct.test01* %st
}
; CHECK: Missing fn type metadata for: test01

define internal void @test02(%struct.test01* %in) {
  ret void
}
; CHECK: Missing fn type metadata for: test02

; Check an alloca instruction
define internal void @test03() {
  %local = alloca %struct.test01*
  %mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %mem to %struct.test01*
  store %struct.test01* %st, %struct.test01** %local
  ret void
}
; CHECK: Missing metadata: test03 :  %local = alloca %struct.test01*

declare noalias i8* @malloc(i64)
; CHECK: Missing fn type metadata for: malloc

!6 = !{i64 0, i32 0}  ; i64
!7 = !{i32 0, i32 0}  ; i32
!8 = !{!"S", %struct.test01  zeroinitializer, i32 3, !6, !7, !7} ; { i64, i32, i32 }

!intel.dtrans.types = !{!8}
