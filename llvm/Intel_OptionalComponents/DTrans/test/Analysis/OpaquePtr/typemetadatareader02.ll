; REQUIRES: asserts

; RUN: opt -opaque-pointers -dtrans-typemetadatareader -dtrans-typemetadatareader-values -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -passes=dtrans-typemetadatareader -dtrans-typemetadatareader-values -disable-output < %s 2>&1 | FileCheck %s

; This test is to check that definitions and instructions that are expected to
; have dtrans type metadata associated with them can be decoded.

%struct.test01 = type { i64, i32, i32 }

; Check a global definition
@test01_var = internal global ptr null, !intel_dtrans_type !1
; CHECK: Decoded var type metadata: @test01_var = internal global ptr null, !intel_dtrans_type !0 - %struct.test01*

; Check a function definition
define internal "intel_dtrans_func_index"="1" ptr @test01() !intel.dtrans.func.type !3 {
  %st = call ptr @malloc(i64 16)
  ret ptr %st
}
; CHECK: Decoded fn type metadata: test01 : %struct.test01* ()

define internal void @test02(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !5 {
  ret void
}
; CHECK: Decoded fn type metadata: test02 : void (%struct.test01*)

; Check an alloca instruction
define internal void @test03() {
  %local = alloca ptr, !intel_dtrans_type !1
  %st = call ptr @malloc(i64 16)
  store ptr %st, ptr %local
  ret void
}
; CHECK: Decoded alloca metadata : test03 :   %local = alloca ptr, align 8, !intel_dtrans_type !0 - %struct.test01*

declare !intel.dtrans.func.type !12 noalias "intel_dtrans_func_index"="1" ptr @malloc(i64)

; CHECK: Decoded fn type metadata: malloc : i8* (i64)

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
