; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test initializing a function pointer field member with an incompatible
; function pointer type. Currently, this test will trigger a safety condition on
; the structure type, because a strict type match check is performed. However,
; this may need to be relaxed in the future to account for initialization of
; virtual function tables.

%struct.test01 = type { i32, void(%struct.test01*)* }
@test01 = internal global %struct.test01{ i32 1, void(%struct.test01*)* bitcast (void (i8*)* @func01 to void(%struct.test01*)*) }
define void @func01(i8* "intel_dtrans_func_index"="1") !intel.dtrans.func.type !7 {
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test01
; CHECK: Safety data: Unsafe pointer store | Global instance | Has initializer list | Has function ptr{{ *$}}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !3, !4}  ; void(%struct.test01*)
!3 = !{!"void", i32 0}  ; void
!4 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!5 = !{!2, i32 1}  ; void(%struct.test01*)*
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !5} ; { i32, void(%struct.test01*)* }

!intel.dtrans.types = !{!8}