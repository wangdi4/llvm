; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery for malloc library calls based on
; inference from uses of the value created by the call instruction.

%struct.testmember01 = type { i64, i64 }
%struct.test01 = type { ptr }
@var_test01 = internal global %struct.test01 zeroinitializer
define internal void @test01() {
  %mystruct = call ptr @malloc(i64 16)

  ; This infers the allocation type based on the type of the location being
  ; stored to.
  store ptr %mystruct, ptr getelementptr (%struct.test01, ptr @var_test01, i64 0, i32 0)
  ret void
}
; CHECK-LABEL: void @test01
; CHECK: %mystruct = call ptr @malloc(i64 16)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:  %struct.testmember01*{{ *$}}
; CHECK-NEXT:  i8*{{ *$}}
; CHECK-NEXT: No element pointees.


; Test an allocation that is not a structure type.
define internal void @test02(i64 %count) {
  %size = mul i64 %count, 8
  %mem_double = call ptr @malloc(i64 %size)
  store double 0.000000e+00, ptr %mem_double
  ret void
}
; CHECK-LABEL: void @test02
; CHECK: %mem_double = call ptr @malloc(i64 %size)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:  double*{{ *$}}
; CHECK-NEXT:  i8*{{ *$}}
; CHECK-NEXT: No element pointees.


declare !intel.dtrans.func.type !4 "intel_dtrans_func_index"="1" ptr @malloc(i64)

!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.testmember01 zeroinitializer, i32 1}  ; %struct.testmember01*
!3 = !{i8 0, i32 1}  ; i8*
!4 = distinct !{!3}
!5 = !{!"S", %struct.testmember01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!6 = !{!"S", %struct.test01 zeroinitializer, i32 1, !2} ; { %struct.testmember01* }

!intel.dtrans.types = !{!5, !6}
