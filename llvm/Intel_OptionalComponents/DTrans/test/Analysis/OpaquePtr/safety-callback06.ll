; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Test callback function which can invoke multiple functions with a specifier
; that identifies which parameters are forwarded to each target function.
; Test case where a parameter is forwarded to more than one function,
; and the type of the target function does not match the expected type.

%struct.test01 = type { i32, i32, i32, i64, i32 }
%struct.test01alt = type { i64, i32, i64, i32 }
%struct.test02 = type { i32, i32 }
define void @test01(ptr "intel_dtrans_func_index"="1" %img, ptr "intel_dtrans_func_index"="2" %s2) !intel.dtrans.func.type !7 {
  tail call void @broker(
    ptr @test01callee,
    ptr @test01callee2,
    ptr %img,
    ptr %s2
  )

  ret void
}

define void @test01callee(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !8 {
  %use1 = getelementptr %struct.test01, ptr %in, i64 0, i32 1
  ret void
}

define void @test01callee2(ptr "intel_dtrans_func_index"="1" %in, ptr "intel_dtrans_func_index"="2" %alt) !intel.dtrans.func.type !10 {
  %use1 = getelementptr %struct.test02, ptr %in, i64 0, i32 1
  %use2 = getelementptr %struct.test01alt, ptr %alt, i64 0, i32 1
  ret void
}

declare !intel.dtrans.func.type !16 !callback !0 void @broker(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", ptr "intel_dtrans_func_index"="3", ptr "intel_dtrans_func_index"="4")

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

; This structure is passed with the expected type, and should not get a safety
; flag.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test02

!0 = !{!1, !2}
!1 = !{i64 0, i64 2, i1 false}
!2 = !{i64 1, i64 3, i64 2, i1 false}
!3 = !{i32 0, i32 0}  ; i32
!4 = !{i64 0, i32 0}  ; i64
!5 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!6 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!7 = distinct !{!5, !6}
!8 = distinct !{!5}
!9 = !{%struct.test01alt zeroinitializer, i32 1}  ; %struct.test01alt*
!10 = distinct !{!6, !9}
!11 = !{!"F", i1 false, i32 1, !12, !5}  ; void (%struct.test01*)
!12 = !{!"void", i32 0}  ; void
!13 = !{!11, i32 1}  ; void (%struct.test01*)*
!14 = !{!"F", i1 false, i32 2, !12, !6, !9}  ; void (%struct.test02*, %struct.test01alt*)
!15 = !{!14, i32 1}  ; void (%struct.test02*, %struct.test01alt*)*
!16 = distinct !{!13, !15, !5, !6}
!17 = !{!"S", %struct.test01 zeroinitializer, i32 5, !3, !3, !3, !4, !3} ; { i32, i32, i32, i64, i32 }
!18 = !{!"S", %struct.test01alt zeroinitializer, i32 4, !4, !3, !4, !3} ; { i64, i32, i64, i32 }
!19 = !{!"S", %struct.test02 zeroinitializer, i32 2, !3, !3} ; { i32, i32 }

!intel.dtrans.types = !{!17, !18, !19}
