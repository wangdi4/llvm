; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases of converting a pointer to an integer for use in a subtract
; instruction that are safe.

; Subtracting two pointers of the same type yields a safe offset when the
; result is used for dividing by the structure's size.
%struct.test01 = type { i64, i64 }
define void @test01(%struct.test01* %pStruct1, %struct.test01* %pStruct2) !dtrans_type !2 {
  %t1 = ptrtoint %struct.test01* %pStruct1 to i64
  %t2 = ptrtoint %struct.test01* %pStruct2 to i64
  %offset = sub i64 %t2, %t1
  ; Division by structure size.
  %count = sdiv i64 %offset, 16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: No issues found

; udiv is also safe.
%struct.test02 = type { i64, i64 }
define void @test02(%struct.test02* %pStruct1, %struct.test02* %pStruct2) !dtrans_type !6 {
  %t1 = ptrtoint %struct.test02* %pStruct1 to i64
  %t2 = ptrtoint %struct.test02* %pStruct2 to i64
  %offset = sub i64 %t2, %t1
  ; Division by structure size.
  %count = udiv i64 %offset, 16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: No issues found


; Subtracting ptr-to-ptr types does not require division by the aggregate type
; size.
%struct.test03 = type { i64, i64 }
define void @test03(%struct.test03** %ppStruct1, %struct.test03** %ppStruct2) !dtrans_type !9 {
  %t1 = ptrtoint %struct.test03** %ppStruct1 to i64
  %t2 = ptrtoint %struct.test03** %ppStruct2 to i64
  %offset = sub i64 %t2, %t1
  ; Division by pointer size, not by structure size.
  %count = sdiv i64 %offset, 8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: Safety data: No issues found

!1 = !{i64 0, i32 0}  ; i64
!2 = !{!"F", i1 false, i32 2, !3, !4, !4}  ; void (%struct.test01*, %struct.test01*)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{!"F", i1 false, i32 2, !3, !7, !7}  ; void (%struct.test02*, %struct.test02*)
!7 = !{!8, i32 1}  ; %struct.test02*
!8 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!9 = !{!"F", i1 false, i32 2, !3, !10, !10}  ; void (%struct.test03**, %struct.test03**)
!10 = !{!11, i32 2}  ; %struct.test03**
!11 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!12 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!13 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!14 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }

!dtrans_types = !{!12, !13, !14}
