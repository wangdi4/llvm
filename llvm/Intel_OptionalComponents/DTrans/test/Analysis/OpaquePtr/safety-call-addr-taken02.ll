; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Calling a function using an alias to the function which appears to be a
; properly typed pointer to the structure. However, this case is not safe
; for DTrans because the alias is defined using a weak alias linkage.
%struct.test01 = type { i32, i32 }
@f01_alias = weak alias void (%struct.test01*), void (%struct.test01*)* @f01

define internal void @f01(%struct.test01* "intel_dtrans_func_index"="1" %s) !intel.dtrans.func.type !3 {
  %p = getelementptr %struct.test01, %struct.test01* %s, i64 0, i32 0
  %i = load i32, i32* %p
  ret void
}

define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %s) !intel.dtrans.func.type !4 {
  call void @f01_alias(%struct.test01* %s)
  ret void
}

; TODO: This case should be marked as "Address taken". However, the
; PointerTypeAnalyzer will mark the arguments to a weakly defined alias as
; "Unhandled", so this will currently be marked as "Unhandled use". We don't
; expect to see "weak aliases" when we have whole program, so the important
; thing is just that they not be treated as safe, if we do.

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Unhandled use{{ *$}}
; CHECK: End LLVMType: %struct.test01

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = distinct !{!2}
!5 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!5}
