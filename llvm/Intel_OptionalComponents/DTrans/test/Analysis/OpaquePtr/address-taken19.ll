; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed  -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed  -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s

; Test two identical structures with different names.
;
; Check that AddressTaken is set on %struct.test01c, as %struct.test01a is a
; compatible type. This is a regression test for CMPLRLLVM-32371, to verify that
; the behavior is deterministic. The lookup for compatible types requires
; walking the alias set of the indirect function call arguments looking at all
; known types that may be compatible. Previously, the TypeInfo object type that
; corresponded to the pointer type used for the parameter of the indirect
; function call was created during this walk of the alias set, which allowed
; missing the safety bit setting because the matching TypeInfo object may not
; have been created yet due to the alias set walk order.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data:{{.*}}Address taken{{.*}}
; CHECK: End LLVMType: %struct.test01a

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01c
; CHECK: Safety data:{{.*}}Address taken{{.*}}
; CHECK: End LLVMType: %struct.test01c

%struct.test01a = type { i32*, i32 }
%struct.test01c = type { i32*, i32 }

declare !intel.dtrans.func.type !4 void @useA(%struct.test01c* "intel_dtrans_func_index"="1")

@var = global void (%struct.test01a*)* zeroinitializer, !intel_dtrans_type !8

define void @test1(%struct.test01c* "intel_dtrans_func_index"="1" %c) !intel.dtrans.func.type !9 {
  %fptr = load void (%struct.test01a*)*, void (%struct.test01a*)** @var
  %a = bitcast %struct.test01c* %c to %struct.test01a*
  call void %fptr(%struct.test01a* %a), !intel_dtrans_type !5
  ret void
}

!1 = !{i32 0, i32 1}  ; i32*
!2 = !{i32 0, i32 0}  ; i32
!3 = !{%struct.test01c zeroinitializer, i32 1}  ; %struct.test01c*
!4 = distinct !{!3}
!5 = !{!"F", i1 false, i32 1, !6, !7}  ; void (%struct.test01a*)
!6 = !{!"void", i32 0}  ; void
!7 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!8 = !{!5, i32 1}  ; void (%struct.test01a*)*
!9 = distinct !{!3}
!10 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !2} ; { i32*, i32 }
!11 = !{!"S", %struct.test01c zeroinitializer, i32 2, !1, !2} ; { i32*, i32 }

!intel.dtrans.types = !{!10, !11}
