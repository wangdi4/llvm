; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test calls made with "invoke". Parameters which are pointers to aggregate types
; should get "Has C++ handling" set.

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
    resume { i8*, i32 } %lp
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Has C++ handling{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Has C++ handling{{ *$}}
; CHECK: End LLVMType: %struct.test01b


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
