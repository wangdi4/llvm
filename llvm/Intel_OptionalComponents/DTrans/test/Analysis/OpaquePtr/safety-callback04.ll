; REQUIRES: asserts
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test callback function with a specifier that identifies which parameters to
; forward to the target function. This is a case where the parameter type does
; not match the expected type, so should trigger a DTrans safety flag.
%struct.test01 = type { i32, i32, i32, i64, i32 }
%struct.test01alt = type { i64, i32, i64, i32 }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %img) !intel.dtrans.func.type !5 {
  tail call void @broker(
    void (%struct.test01alt*)* @test01callee,
    %struct.test01* %img
  )
  ret void
}

define void @test01callee(%struct.test01alt* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !7 {
  %use1 = getelementptr %struct.test01alt, %struct.test01alt* %in, i64 0, i32 1
  ret void
}

declare !intel.dtrans.func.type !11 !callback !0 void @broker(void (%struct.test01alt*)* "intel_dtrans_func_index"="1", %struct.test01* "intel_dtrans_func_index"="2")

; These should get "Mismatched argument use" because the type passed to the
; broker function does not match the type of the function the broker function
; will call.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Mismatched argument use
; CHECK: End LLVMType: %struct.test01

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01alt
; CHECK: Safety data: Mismatched argument use
; CHECK: End LLVMType: %struct.test01alt

!0 = !{!1}
!1 = !{i64 0, i64 1, i1 false}
!2 = !{i32 0, i32 0}  ; i32
!3 = !{i64 0, i32 0}  ; i64
!4 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!5 = distinct !{!4}
!6 = !{%struct.test01alt zeroinitializer, i32 1}  ; %struct.test01alt*
!7 = distinct !{!6}
!8 = !{!"F", i1 false, i32 1, !9, !6}  ; void (%struct.test01alt*)
!9 = !{!"void", i32 0}  ; void
!10 = !{!8, i32 1}  ; void (%struct.test01alt*)*
!11 = distinct !{!10, !4}
!12 = !{!"S", %struct.test01 zeroinitializer, i32 5, !2, !2, !2, !3, !2} ; { i32, i32, i32, i64, i32 }
!13 = !{!"S", %struct.test01alt zeroinitializer, i32 4, !3, !2, !3, !2} ; { i64, i32, i64, i32 }

!intel.dtrans.types = !{!12, !13}
