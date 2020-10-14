; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test calling a function using an alias to the function using a properly typed
; pointer to the structure. This is safe because we have the function's
; definition and the types match for the caller and callee.

%struct.test01 = type { i32, i32 }
@f01_alias = internal alias void (%struct.test01*), void (%struct.test01*)* @f01

define internal void @f01(%struct.test01* %s) !dtrans_type !2 {
  %p = getelementptr %struct.test01, %struct.test01* %s, i64 0, i32 0
  store i32 0, i32* %p
  ret void
}

define void @test01(%struct.test01* %s) !dtrans_type !2 {
  call void @f01_alias(%struct.test01* %s)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: No issues found


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !3, !4}  ; void (%struct.test01*)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!6}
