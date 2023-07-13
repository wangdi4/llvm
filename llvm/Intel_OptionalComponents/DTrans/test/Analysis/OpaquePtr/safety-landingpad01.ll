; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -debug-only=dtransanalysis -disable-output %s 2>&1 | FileCheck %s

; Test that the DTrans safety analyzer does not trigger safety settings on
; landingpad, extractvalue, insertvalue or resume instructions.

%struct.test01a = type { i32, i32 }
%struct.test01b = type { i32, i32 }
define i32 @test01(ptr "intel_dtrans_func_index"="1" %pStructA, ptr "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !4 {
  %addr1 = getelementptr %struct.test01a, ptr %pStructA, i64 0, i32 0
  %addr2 = getelementptr %struct.test01b, ptr %pStructB, i64 0, i32 0
  %val1 = load i32, ptr %addr1
  %val2 = load i32, ptr %addr2
  %add = add i32 %val1, %val2
  ret i32 %add
}

define void @test01i(ptr "intel_dtrans_func_index"="1" %s1, ptr "intel_dtrans_func_index"="2" %s2) personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !5 {
  %sum = invoke i32 @test01(ptr %s1, ptr %s2) to label %good unwind label %bad
good:
  ret void

bad:
  %lp = landingpad { ptr, i32 }
        cleanup
  %ev1 = extractvalue { ptr, i32 } %lp, 0
  %ev2 = extractvalue { ptr, i32 } %lp, 1
  %iv1 = insertvalue { ptr, i32 } undef, ptr %ev1, 0
  %iv2 = insertvalue { ptr, i32 } %iv1, i32 %ev2, 1
  resume { ptr, i32 } %iv2
}

; CHECK: dtrans-safety-detail: %struct.test01a = type { i32, i32 } :: Has C++ handling
; CHECK: dtrans-safety-detail: %struct.test01b = type { i32, i32 } :: Has C++ handling

; There should not be any "Unhandled use"" messages.
; CHECK-NOT: Unhandled use

declare i32 @__gxx_personality_v0(...)
declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" ptr @_Znwm(i64)

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

