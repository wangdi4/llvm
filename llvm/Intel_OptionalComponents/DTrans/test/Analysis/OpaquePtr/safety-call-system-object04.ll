; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test handling of return value type from indirect function calls results in
; them being marked with "System object".

%struct.test01 = type { i32, i32 }
@var01 = internal global %struct.test01* zeroinitializer, !dtrans_type !2
@func01 = internal global %struct.test01* (i32)* @test01i, !dtrans_type !5
define void @test01() {
  %funcaddr = load %struct.test01* (i32)*, %struct.test01* (i32)** @func01
  %pStruct = call %struct.test01* (i32) %funcaddr(i32 1), !dtrans_type !4
  ret void
}

define %struct.test01* @test01i(i32 %x) !dtrans_type !4 {
  %val = load %struct.test01*, %struct.test01** @var01
  ret %struct.test01* %val
}
; This could probably be treated as safe, but the original LocalPointerAnalysis
; implementation of DTrans marked any return types from indirect function calls
; as "System object". To handle this would require extending the analysis to
; resolve that there are no external functions that are address taken which
; match the signature of the indirect call.

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: Global pointer | System object{{ *$}}


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!3, i32 1}  ; %struct.test01*
!3 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!4 = !{!"F", i1 false, i32 1, !2, !1}  ; %struct.test01* (i32)
!5 = !{!4, i32 1}  ; %struct.test01* (i32)*
!6 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!6}
