; REQUIRES: asserts

; RUN: opt -dtrans-typemetadatareader -debug-only=dtrans-typemetadatareader -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes=dtrans-typemetadatareader -debug-only=dtrans-typemetadatareader -disable-output < %s 2>&1 | FileCheck %s

; This test is to check that definitions and instructions that are expected to
; have dtrans type metadata associated with them can be decoded.

%struct.test01 = type { i64, i32, i32 }

; Check a global definition
@test01_var = internal global %struct.test01* null, !dtrans_type !1
; CHECK-LABEL: Checking module for DTrans metadata
; CHECK: Decoded var type metadata: @test01_var = internal global %struct.test01* null, !dtrans_type !0 - %struct.test01*

; Check a function definition
define internal %struct.test01* @test01() !dtrans_type !3 {
  %mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %mem to %struct.test01*
  ret %struct.test01* %st
}
; CHECK: Checking function for DTrans metadata: test01
; CHECK: Decoded fn type metadata:  - %struct.test01* ()

define internal void @test02(%struct.test01* %in) !dtrans_type !4 {
  ret void
}
; CHECK: Checking function for DTrans metadata: test02
; CHECK: Decoded fn type metadata:  - void (%struct.test01*)

; Check an alloca instruction
define internal void @test03() {
  %local = alloca %struct.test01*, !dtrans_type !1
  %mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %mem to %struct.test01*
  store %struct.test01* %st, %struct.test01** %local
  ret void
}
; CHECK: Checking function for DTrans metadata: test03
; CHECK: Decoded metadata:   %local = alloca %struct.test01*, align 8, !dtrans_type !0 - %struct.test01*

declare noalias i8* @malloc(i64)

!1 = !{!2, i32 1}  ; %struct.test01*
!2 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!3 = !{!"F", i1 false, i32 0, !1}  ; %struct.test01* ()
!4 = !{!"F", i1 false, i32 1, !5, !1}  ; void (%struct.test01*)
!5 = !{!"void", i32 0}  ; void
!6 = !{i64 0, i32 0}  ; i64
!7 = !{i32 0, i32 0}  ; i32
!8 = !{!"S", %struct.test01  zeroinitializer, i32 3, !6, !7, !7} ; { i64, i32, i32 }

!dtrans_types = !{!8}
