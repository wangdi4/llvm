; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery on getelementptr instructions

%struct.test01 = type { i64, %struct.test01* }

define void @test01(%struct.test01* %in)  !dtrans_type !1 {
  %f0 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 0
  %v0 = load i64, i64* %f0

  %f1 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
  %v1 = load %struct.test01*, %struct.test01** %f1

  ret void
}

; TODO: Currently, only the framework exists for walking the elements to print,
; so all pointers are reported as not having information. When the analysis is
; implemented the real types and field accesses should be reported.


; CHECK: Input Parameters:
; CHECK:  0: %struct.test01* %in
; CHECK:        <NO PTR INFO AVAILABLE>
; CHECK: void @test01(%struct.test01* %in) !dtrans_type !4 {
; CHECK: %f0 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 0
; CHECK:            <NO PTR INFO AVAILABLE>

; CHECK: %v0 = load i64, i64* %f0
; CHECK: %f1 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
; CHECK:            <NO PTR INFO AVAILABLE>

; CHECK: %v1 = load %struct.test01*, %struct.test01** %f1
; CHECK:            <NO PTR INFO AVAILABLE>


!1 = !{!"F", i1 false, i32 1, !2, !3}  ; void (%struct.test01*)
!2 = !{!"void", i32 0}  ; void
!3 = !{!4, i32 1}  ; %struct.test01*
!4 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!5 = !{i64 0, i32 0}  ; i64
!6 = !{!"S", %struct.test01  zeroinitializer, i32 2, !5, !3} ; { i64, %struct.test01* }

!dtrans_types = !{!6}
