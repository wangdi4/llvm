; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type recovery on constant operator expressions

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

%struct.test01 = type { i64, double, [25 x i8] }
@test_var01 = internal global %struct.test01 zeroinitializer

; Test GEPOperator processing
define internal void @test01() {
  %v0 = load i64, i64* getelementptr (%struct.test01, %struct.test01* @test_var01, i64 0, i32 0)
  %v1 = load double, double* getelementptr (%struct.test01, %struct.test01* @test_var01, i64 0, i32 1)
  %v2 = load i8, i8* getelementptr (%struct.test01, %struct.test01* @test_var01, i64 0, i32 2, i32 0)
  ret void
}
; CHECK-LABEL: void @test01() {
; CHECK-CUR: %v0 = load i64, i64* getelementptr inbounds (%struct.test01, %struct.test01* @test_var01, i64 0, i32 0)
; CHECK-FUT: %v0 = load i64, p0 getelementptr inbounds (%struct.test01, p0 @test_var01, i64 0, i32 0)
; CHECK-CUR:         CE: i64* getelementptr inbounds (%struct.test01, %struct.test01* @test_var01, i64 0, i32 0)
; CHECK-FUT:         CE: p0 getelementptr inbounds (%struct.test01, p0 @test_var01, i64 0, i32 0)
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i64*{{ *$}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test01 @ 0

; CHECK-CUR:  %v1 = load double, double* getelementptr inbounds (%struct.test01, %struct.test01* @test_var01, i64 0, i32 1)
; CHECK-FUT:  %v1 = load double, p0 getelementptr inbounds (%struct.test01, p0 @test_var01, i64 0, i32 1)
; CHECK-CUR:         CE: double* getelementptr inbounds (%struct.test01, %struct.test01* @test_var01, i64 0, i32 1)
; CHECK-FUT:         CE: p0 getelementptr inbounds (%struct.test01, p0 @test_var01, i64 0, i32 1)
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        double*{{ *$}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test01 @ 1

; CHECK-CUR:  %v2 = load i8, i8* getelementptr inbounds (%struct.test01, %struct.test01* @test_var01, i64 0, i32 2, i32 0)
; CHECK-FUT:  %v2 = load i8, p0 getelementptr inbounds (%struct.test01, p0 @test_var01, i64 0, i32 2, i32 0)
; CHECK-CUR:         CE: i8* getelementptr inbounds (%struct.test01, %struct.test01* @test_var01, i64 0, i32 2, i32 0)
; CHECK-FUT:         CE: p0 getelementptr inbounds (%struct.test01, p0 @test_var01, i64 0, i32 2, i32 0)
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i8*{{ *$}}
; CHECK-NEXT:     Element pointees:
; CHECK-NEXT:        [25 x i8] @ 0

; Test that an unhandled constant expression operator, PtrToIntOperator in
; this case, gets marked as UNHANDLED.
%struct.test02 = type { i64*, %struct.test02* }
@test_var02 = internal global %struct.test02 zeroinitializer
define internal void @test02() {
  %local = alloca i64
  store i64 ptrtoint (%struct.test02** getelementptr (%struct.test02, %struct.test02* @test_var02, i64 0, i32 1) to i64), i64* %local
  ret void
}
; CHECK-LABEL: void @test02() {
; CHECK-CUR:  store i64 ptrtoint (%struct.test02** getelementptr inbounds (%struct.test02, %struct.test02* @test_var02, i64 0, i32 1) to i64), i64* %local
; CHECK-FUT:  store i64 ptrtoint (p0 getelementptr inbounds (%struct.test02, p0 @test_var02, i64 0, i32 1) to i64), p0 %local
; CHECK-CUR:     CE: i64 ptrtoint (%struct.test02** getelementptr inbounds (%struct.test02, %struct.test02* @test_var02, i64 0, i32 1) to i64)
; CHECK-FUT:     CE: i64 ptrtoint (p0 getelementptr inbounds (%struct.test02, p0 @test_var02, i64 0, i32 1) to i64)
; CHECK-NEXT:       LocalPointerInfo:
; CHECK-SAME: <UNHANDLED>

; CHECK-CUR:     CE: %struct.test02** getelementptr inbounds (%struct.test02, %struct.test02* @test_var02, i64 0, i32 1)
; CHECK-FUT:     CE: p0 getelementptr inbounds (%struct.test02, p0 @test_var02, i64 0, i32 1)
; CHECK-NEXT:       LocalPointerInfo:
; CHECK-NEXT:          Aliased types:
; CHECK-NEXT:            %struct.test02**{{ *$}}
; CHECK-NEXT:          Element pointees:
; CHECK-NEXT:            %struct.test02 @ 1


!1 = !{i64 0, i32 0}  ; i64
!2 = !{double 0.0e+00, i32 0}  ; double
!3 = !{!"A", i32 25, !4}  ; [25 x i8]
!4 = !{i8 0, i32 0}  ; i8
!5 = !{i64 0, i32 1}  ; i64*
!6 = !{!7, i32 1}  ; %struct.test02*
!7 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!8 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !3} ; { i64, double, [25 x i8] }
!9 = !{!"S", %struct.test02 zeroinitializer, i32 2, !5, !6} ; { i64*, %struct.test02* }

!dtrans_types = !{!8, !9}
