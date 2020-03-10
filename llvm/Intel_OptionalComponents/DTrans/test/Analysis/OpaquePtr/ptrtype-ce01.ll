; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery on constant operator expressions

%struct.test01 = type { i64, double, [25 x i8] }
@test_var01 = internal global %struct.test01 zeroinitializer

; Test GEPOperator processing
define internal void @test01() {
  %v0 = load i64, i64* getelementptr (%struct.test01, %struct.test01* @test_var01, i64 0, i32 0)
  %v1 = load double, double* getelementptr (%struct.test01, %struct.test01* @test_var01, i64 0, i32 1)
  %v2 = load i8, i8* getelementptr (%struct.test01, %struct.test01* @test_var01, i64 0, i32 2, i32 0)

  ret void
}

; TODO: Currently, only the framework exists for walking the elements to print,
; so all pointers are reported as not having information. When the analysis is
; implemented the real types and field accesses should be reported.

; CHECK-LABEL: void @test01() {
; CHECK: %v0 = load i64, i64* getelementptr inbounds (%struct.test01, %struct.test01* @test_var01, i64 0, i32 0)
; CHECK:         CE: i64* getelementptr inbounds (%struct.test01, %struct.test01* @test_var01, i64 0, i32 0)
; CHECK:            <NO PTR INFO AVAILABLE FOR ConstantExpr>
; CHECK:  %v1 = load double, double* getelementptr inbounds (%struct.test01, %struct.test01* @test_var01, i64 0, i32 1)
; CHECK:         CE: double* getelementptr inbounds (%struct.test01, %struct.test01* @test_var01, i64 0, i32 1)
; CHECK:            <NO PTR INFO AVAILABLE FOR ConstantExpr>
; CHECK:  %v2 = load i8, i8* getelementptr inbounds (%struct.test01, %struct.test01* @test_var01, i64 0, i32 2, i32 0)
; CHECK:         CE: i8* getelementptr inbounds (%struct.test01, %struct.test01* @test_var01, i64 0, i32 2, i32 0)
; CHECK:            <NO PTR INFO AVAILABLE FOR ConstantExpr>

!1 = !{i64 0, i32 0}  ; i64
!2 = !{double 0.0e+00, i32 0}  ; double
!3 = !{!"A", i32 25, !4}  ; [25 x i8]
!4 = !{i8 0, i32 0}  ; i8
!5 = !{!"S", %struct.test01  zeroinitializer, i32 3, !1, !2, !3} ; { i64, double, [25 x i8] }

!dtrans_types = !{!5}
