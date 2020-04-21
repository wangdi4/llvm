; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type recovery for malloc library calls based on
; inference from uses of the value created by the call instruction.

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

%struct.testmember01 = type { i64, i64 }
%struct.test01 = type { %struct.testmember01* }
@var_test01 = internal global %struct.test01 zeroinitializer
define internal void @test01() {
  %mem_i8 = call i8* @malloc(i64 16)
  %mystruct = bitcast i8* %mem_i8 to %struct.testmember01*

  ; This infers the allocation type based on the type of the location being
  ; stored to.
  store %struct.testmember01* %mystruct, %struct.testmember01** getelementptr (%struct.test01, %struct.test01* @var_test01, i64 0, i32 0)
  ret void
}
; CHECK-LABEL: void @test01
; CHECK-CUR: %mem_i8 = call i8* @malloc(i64 16)
; CHECK-FUT: %mem_i8 = call p0 @malloc(i64 16)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:  %struct.testmember01*{{ *$}}
; CHECK-NEXT:  i8*{{ *$}}
; CHECK-NEXT: No element pointees.


; Test an allocation that is not a structure type.
define internal void @test02(i64 %count) {
  %size = mul i64 %count, 8
  %mem_i8 = call i8* @malloc(i64 %size)
  %mem_double = bitcast i8* %mem_i8 to double*
  store double 0.000000e+00, double* %mem_double
  ret void
}
; CHECK-LABEL: void @test02
; CHECK-CUR: %mem_i8 = call i8* @malloc(i64 %size)
; CHECK-FUT: %mem_i8 = call p0 @malloc(i64 %size)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:  double*{{ *$}}
; CHECK-NEXT:  i8*{{ *$}}
; CHECK-NEXT: No element pointees.


declare i8* @malloc(i64)

!1 = !{i64 0, i32 0}  ; i64
!2 = !{!3, i32 1}  ; %struct.testmember01*
!3 = !{!"R", %struct.testmember01 zeroinitializer, i32 0}  ; %struct.testmember01
!4 = !{!"S", %struct.testmember01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!5 = !{!"S", %struct.test01 zeroinitializer, i32 1, !2} ; { %struct.testmember01* }

!dtrans_types = !{!4, !5}
