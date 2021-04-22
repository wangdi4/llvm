; REQUIRES: asserts

; RUN: opt -dtrans-typemetadatareader -debug-only=dtrans-typemetadatareader -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes=dtrans-typemetadatareader -debug-only=dtrans-typemetadatareader -disable-output < %s 2>&1 | FileCheck %s

; This test is to check that definitions and instructions that are expected to
; have dtrans type metadata associated with them can be decoded.

%struct.test01 = type { i64, i32, i32 }

; Check a global definition
@test01_var = internal global %struct.test01* null, !intel_dtrans_type !1
; CHECK-LABEL: Checking module for DTrans metadata
; CHECK: Decoded var type metadata: @test01_var = internal global %struct.test01* null, !intel_dtrans_type !0 - %struct.test01*

; Check a function definition
define internal "intel_dtrans_func_index"="1" %struct.test01* @test01() !intel.dtrans.func.type !3 {
  %mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %mem to %struct.test01*
  ret %struct.test01* %st
}
; CHECK: Checking function for DTrans metadata: test01
; CHECK-NEXT: Decoded fn type metadata: %struct.test01* ()

define internal void @test02(%struct.test01* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !5 {
  ret void
}
; CHECK: Checking function for DTrans metadata: test02
; CHECK-NEXT: Decoded fn type metadata: void (%struct.test01*)

; Check an alloca instruction
define internal void @test03() {
  %local = alloca %struct.test01*, !intel_dtrans_type !1
  %mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %mem to %struct.test01*
  store %struct.test01* %st, %struct.test01** %local
  ret void
}
; CHECK: Checking function for DTrans metadata: test03
; CHECK: Decoded metadata:   %local = alloca %struct.test01*, align 8, !intel_dtrans_type !0 - %struct.test01*

declare !intel.dtrans.func.type !12 noalias "intel_dtrans_func_index"="1" i8* @malloc(i64)

; CHECK: Checking function for DTrans metadata: malloc
; CHECK-NEXT: Decoded fn type metadata: i8* (i64)

!1 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!1}
!5 = distinct !{!1}
!6 = !{!"void", i32 0}  ; void
!7 = !{i64 0, i32 0}  ; i64
!8 = !{i32 0, i32 0}  ; i32
!9 = !{!"S", %struct.test01  zeroinitializer, i32 3, !7, !8, !8} ; { i64, i32, i32 }
!10 = !{i8 0, i32 1}  ; i8*
!12 = distinct !{!10}

!intel.dtrans.types = !{!9}
