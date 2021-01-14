; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test calls made with "invoke". Parameters which are pointers to aggregate types
; should get "Has C++ handling" set.

%struct.test01a = type { i32, i32 }
%struct.test01b = type { i32, i32 }
define i32 @test01(%struct.test01a* %pStructA, %struct.test01b* %pStructB) !dtrans_type !2 {
  %addr1 = getelementptr %struct.test01a, %struct.test01a* %pStructA, i64 0, i32 0
  %addr2 = getelementptr %struct.test01b, %struct.test01b* %pStructB, i64 0, i32 0
  %val1 = load i32, i32* %addr1
  %val2 = load i32, i32* %addr2
  %add = add i32 %val1, %val2
  ret i32 %add
}

define void @test01i(%struct.test01a* %s1, %struct.test01b* %s2) personality i32 (...)* @__gxx_personality_v0 !dtrans_type !7 {
  %sum = invoke i32 @test01(%struct.test01a* %s1, %struct.test01b* %s2) to label %good unwind label %bad
good:
  ret void

bad:
    %lp = landingpad { i8*, i32 }
          cleanup
    resume { i8*, i32 } %lp
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Has C++ handling{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Has C++ handling{{ *$}}


declare i32 @__gxx_personality_v0(...)
declare i8* @_Znwm(i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 2, !1, !3, !5}  ; i32 (%struct.test01a*, %struct.test01b*)
!3 = !{!4, i32 1}  ; %struct.test01a*
!4 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!5 = !{!6, i32 1}  ; %struct.test01b*
!6 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!7 = !{!"F", i1 false, i32 2, !8, !3, !5}  ; void (%struct.test01a*, %struct.test01b*)
!8 = !{!"void", i32 0}  ; void
!9 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!10 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!9, !10}
