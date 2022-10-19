; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

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

define internal void @f01(%struct.test01b* "intel_dtrans_func_index"="1" %s) !intel.dtrans.func.type !3 {
  %p = getelementptr %struct.test01b, %struct.test01b* %s, i64 0, i32 0
  %i = load i32, i32* %p
  ret void
}

define void @test01(%struct.test01a* "intel_dtrans_func_index"="1" %s) !intel.dtrans.func.type !5 {
  %p = getelementptr %struct.test01a, %struct.test01a* %s, i64 0, i32 0
  %i = load i32, i32* %p
  call void @f01_alias(%struct.test01a* %s)
  ret void
}
; NOTE: These also get "Ambiguous GEP" from %p within @test01 being collected
;       as being used as both %struct.test01a and %struct.test01b* types due to
;       the GEP analysis considering all types from the usage set.

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Bad casting | Ambiguous GEP{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Bad casting | Ambiguous GEP{{ *$}}
; CHECK: End LLVMType: %struct.test01b


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01b zeroinitializer, i32 1}  ; %struct.test01b*
!3 = distinct !{!2}
!4 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!5 = distinct !{!4}
!6 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!7 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!6, !7}
