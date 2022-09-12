; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -dtrans-safetyanalyzer -debug-only=dtransanalysis -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -debug-only=dtransanalysis -disable-output %s 2>&1 | FileCheck %s

; Test that the DTrans safety analyzer does not trigger safety settings on
; landingpad, extractvalue, insertvalue or resume instructions.

%struct.test01a = type { i32, i32 }
%struct.test01b = type { i32, i32 }
define i32 @test01(%struct.test01a* "intel_dtrans_func_index"="1" %pStructA, %struct.test01b* "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !4 {
  %addr1 = getelementptr %struct.test01a, %struct.test01a* %pStructA, i64 0, i32 0
  %addr2 = getelementptr %struct.test01b, %struct.test01b* %pStructB, i64 0, i32 0
  %val1 = load i32, i32* %addr1
  %val2 = load i32, i32* %addr2
  %add = add i32 %val1, %val2
  ret i32 %add
}

define void @test01i(%struct.test01a* "intel_dtrans_func_index"="1" %s1, %struct.test01b* "intel_dtrans_func_index"="2" %s2) personality i32 (...)* @__gxx_personality_v0 !intel.dtrans.func.type !5 {
  %sum = invoke i32 @test01(%struct.test01a* %s1, %struct.test01b* %s2) to label %good unwind label %bad
good:
  ret void

bad:
  %lp = landingpad { i8*, i32 }
        cleanup
  %ev1 = extractvalue { i8*, i32 } %lp, 0
  %ev2 = extractvalue { i8*, i32 } %lp, 1
  %iv1 = insertvalue { i8*, i32 } undef, i8* %ev1, 0
  %iv2 = insertvalue { i8*, i32 } %iv1, i32 %ev2, 1
  resume { i8*, i32 } %iv2
}

; CHECK: dtrans-safety-detail: %struct.test01a = type { i32, i32 } :: Has C++ handling
; CHECK: dtrans-safety-detail: %struct.test01b = type { i32, i32 } :: Has C++ handling

; There should not be any "Unhandled use"" messages.
; CHECK-NOT: Unhandled use

declare i32 @__gxx_personality_v0(...)
declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" i8* @_Znwm(i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!3 = !{%struct.test01b zeroinitializer, i32 1}  ; %struct.test01b*
!4 = distinct !{!2, !3}
!5 = distinct !{!2, !3}
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!9 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!8, !9}

