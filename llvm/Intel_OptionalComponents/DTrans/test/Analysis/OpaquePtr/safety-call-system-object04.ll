; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test handling of return value type from indirect function calls results in
; them being marked with "System object".

%struct.test01 = type { i32, i32 }
@var01 = internal global %struct.test01* zeroinitializer, !intel_dtrans_type !2
@func01 = internal global %struct.test01* (i32)* @test01i, !intel_dtrans_type !4
define void @test01() {
  %funcaddr = load %struct.test01* (i32)*, %struct.test01* (i32)** @func01
  %pStruct = call %struct.test01* (i32) %funcaddr(i32 1), !intel_dtrans_type !3
  ret void
}

define "intel_dtrans_func_index"="1" %struct.test01* @test01i(i32 %x) !intel.dtrans.func.type !5 {
  %val = load %struct.test01*, %struct.test01** @var01
  ret %struct.test01* %val
}
; This could probably be treated as safe, but the original LocalPointerAnalysis
; implementation of DTrans marked any return types from indirect function calls
; as "System object". To handle this would require extending the analysis to
; resolve that there are no external functions that are address taken which
; match the signature of the indirect call.

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Global pointer | System object{{ *$}}
; CHECK: End LLVMType: %struct.test01


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{!"F", i1 false, i32 1, !2, !1}  ; %struct.test01* (i32)
!4 = !{!3, i32 1}  ; %struct.test01* (i32)*
!5 = distinct !{!2}
!6 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!6}
