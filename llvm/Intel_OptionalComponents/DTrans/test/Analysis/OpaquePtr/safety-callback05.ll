; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Test callback function which can invoke multiple functions with a specifier
; that identifies which parameters are forwarded to each target function.

%struct.test01 = type { i32, i32, i32, i64, i32 }
%struct.test02 = type { i32, i32 }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %img, %struct.test02* "intel_dtrans_func_index"="2" %s2) !intel.dtrans.func.type !7 {
  tail call void @broker(
    void (%struct.test01*)* @test01callee,
	void (%struct.test02*)* @test01callee2,
    %struct.test01* %img,
    %struct.test02* %s2
  )

  ret void
}

define void @test01callee(%struct.test01* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !8 {
  %use1 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
  ret void
}

define void @test01callee2(%struct.test02* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !9 {
  %use1 = getelementptr %struct.test02, %struct.test02* %in, i64 0, i32 1
  ret void
}

declare !intel.dtrans.func.type !15 !callback !0 void @broker(void (%struct.test01*)* "intel_dtrans_func_index"="1", void (%struct.test02*)* "intel_dtrans_func_index"="2", %struct.test01* "intel_dtrans_func_index"="3", %struct.test02* "intel_dtrans_func_index"="4")

; These structures should not get marked "Address taken" because they are passed
; to the external broker function to be forwarded to the callback function.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test02

!0 = !{!1, !2}
!1 = !{i64 0, i64 2, i1 false}
!2 = !{i64 1, i64 3, i1 false}
!3 = !{i32 0, i32 0}  ; i32
!4 = !{i64 0, i32 0}  ; i64
!5 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!6 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!7 = distinct !{!5, !6}
!8 = distinct !{!5}
!9 = distinct !{!6}
!10 = !{!"F", i1 false, i32 1, !11, !5}  ; void (%struct.test01*)
!11 = !{!"void", i32 0}  ; void
!12 = !{!10, i32 1}  ; void (%struct.test01*)*
!13 = !{!"F", i1 false, i32 1, !11, !6}  ; void (%struct.test02*)
!14 = !{!13, i32 1}  ; void (%struct.test02*)*
!15 = distinct !{!12, !14, !5, !6}
!16 = !{!"S", %struct.test01 zeroinitializer, i32 5, !3, !3, !3, !4, !3} ; { i32, i32, i32, i64, i32 }
!17 = !{!"S", %struct.test02 zeroinitializer, i32 2, !3, !3} ; { i32, i32 }

!intel.dtrans.types = !{!16, !17}
