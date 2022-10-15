; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test calls made with "invoke" that return pointers to aggregate types
; that do not cause safety flags, other than "Has C++ handling", to be set.

%struct.test01 = type { i32, i32 }
define "intel_dtrans_func_index"="1" %struct.test01* @test01(%struct.test01* "intel_dtrans_func_index"="2" %pStruct, i64 %idx) !intel.dtrans.func.type !3 {
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
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Has C++ handling{{ *$}}
; CHECK: End LLVMType: %struct.test01


declare i32 @__gxx_personality_v0(...)
declare !intel.dtrans.func.type !5 "intel_dtrans_func_index"="1" i8* @_Znwm(i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2, !2}
!4 = !{i8 0, i32 1}  ; i8*
!5 = distinct !{!4}
!6 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!6}
