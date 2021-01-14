; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test calls made with "invoke" that return pointers to aggregate types
; that do not cause safety flags, other than "Has C++ handling", to be set.

%struct.test01 = type { i32, i32 }
define %struct.test01* @test01(%struct.test01* %pStruct, i64 %idx) !dtrans_type !2 {
  %res = getelementptr %struct.test01, %struct.test01* %pStruct, i64 %idx
  ret %struct.test01* %res
}

define void @test01i() personality i32 (...)* @__gxx_personality_v0 {
  %mem = invoke i8* @_Znwm(i64 8)
            to label %good unwind label %bad
good:
  %head = bitcast i8* %mem to %struct.test01*
  %elem = invoke %struct.test01* @test01(%struct.test01* %head, i64 0)
            to label %good2 unwind label %bad
good2:
  ret void

bad:
    %lp = landingpad { i8*, i32 }
          cleanup
    resume { i8*, i32 } %lp
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: Has C++ handling{{ *$}}


declare i32 @__gxx_personality_v0(...)
declare i8* @_Znwm(i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 2, !3, !3, !5}  ; %struct.test01* (%struct.test01*, i64)
!3 = !{!4, i32 1}  ; %struct.test01*
!4 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!5 = !{i64 0, i32 0}  ; i64
!6 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!6}
