; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Test callback function with a specifier that identifies which parameters to
; forward to the target function, and does not forward all parameters.

%struct.test01 = type { i32, i32, i32, i64, i32 }
%struct.test02 = type { i32, i32 }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %img, %struct.test02* "intel_dtrans_func_index"="2" %s2) !intel.dtrans.func.type !6 {
  tail call void @broker(
    void (%struct.test01*)* @test01callee,
    %struct.test01* %img,
    %struct.test02* %s2
  )

  ret void
}

define void @test01callee(%struct.test01* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !7 {
  %use1 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
  ret void
}

declare !intel.dtrans.func.type !11 !callback !0 void @broker(void (%struct.test01*)* "intel_dtrans_func_index"="1", %struct.test01* "intel_dtrans_func_index"="2", %struct.test02* "intel_dtrans_func_index"="3")

; This structure should not get marked "Address taken" because it is passed to
; the external broker function to be forwarded to the callback function, and the
; broker function is not permitted to inspect or modify those parameters.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01

; This structure is not forwarded by the broker function, so should be marked as
; "Address taken".

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Address taken
; CHECK: End LLVMType: %struct.test02

!0 = !{!1}
!1 = !{i64 0, i64 1, i1 false}
!2 = !{i32 0, i32 0}  ; i32
!3 = !{i64 0, i32 0}  ; i64
!4 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!5 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!6 = distinct !{!4, !5}
!7 = distinct !{!4}
!8 = !{!"F", i1 false, i32 1, !9, !4}  ; void (%struct.test01*)
!9 = !{!"void", i32 0}  ; void
!10 = !{!8, i32 1}  ; void (%struct.test01*)*
!11 = distinct !{!10, !4, !5}
!12 = !{!"S", %struct.test01 zeroinitializer, i32 5, !2, !2, !2, !3, !2} ; { i32, i32, i32, i64, i32 }
!13 = !{!"S", %struct.test02 zeroinitializer, i32 2, !2, !2} ; { i32, i32 }

!intel.dtrans.types = !{!12, !13}
