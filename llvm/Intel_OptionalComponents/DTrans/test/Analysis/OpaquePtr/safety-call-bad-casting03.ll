; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Call a function using a bitcast alias to a function using a different type
; than expected.

; This case should set the "Bad casting" safety bit, because it
; should behave equivalent to the call:
;   call void bitcast (void (%struct.test01b*)* @f01 to
;                      void (%struct.test01a*)*)(%struct.test01a* %s)
;
%struct.test01a = type { i32, i32 }
%struct.test01b = type { i32, i32 }
@f01_alias = internal alias void (%struct.test01a*), bitcast (void (%struct.test01b*)* @f01 to void (%struct.test01a*)*)

define internal void @f01(%struct.test01b* %s) !dtrans_type !2 {
  %p = getelementptr %struct.test01b, %struct.test01b* %s, i64 0, i32 0
  %i = load i32, i32* %p
  ret void
}

define void @test01(%struct.test01a* %s) !dtrans_type !6 {
  %p = getelementptr %struct.test01a, %struct.test01a* %s, i64 0, i32 0
  %i = load i32, i32* %p
  call void @f01_alias(%struct.test01a* %s)
  ret void
}
; NOTE: These also get "Ambiguous GEP" from %p within @test01 being collected
;       as being used as both %struct.test01a and %struct.test01b* types due to
;       the GEP analysis considering all types from the usage set.

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Bad casting | Ambiguous GEP{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Bad casting | Ambiguous GEP{{ *$}}


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !3, !4}  ; void (%struct.test01b*)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01b*
!5 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!6 = !{!"F", i1 false, i32 1, !3, !7}  ; void (%struct.test01a*)
!7 = !{!8, i32 1}  ; %struct.test01a*
!8 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!9 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!10 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!9, !10}
