; Test that the DTransSafetyAnalyzer handles a case where the PtrTypeAnalyzer
; collects an element pointee that is an element of a vector type where the
; pointer type in the vector does NOT match the pointer type it is used as.

; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

%struct.test01 = type { float, float }
%struct.test02 = type { i8, <2 x ptr>, i32 }


define double @test(%struct.test02* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !7 {
  %a = getelementptr %struct.test02, ptr %in, i64 0, i32 1
  %b = getelementptr <2 x ptr>, ptr %a, i64 0, i64 1
  %test01ptr = load ptr, ptr %b
  %ret = load double, ptr %test01ptr
  ret double %ret
}

; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test01 = type { float, float }
  
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test02

!intel.dtrans.types = !{!9, !10}

!1 = !{double 0.0e+00, i32 0}  ; double
!2 = !{float 0.0e+00, i32 0}  ; float
!3 = !{i8 0, i32 0}  ; i8
!4 = !{!"V", i32 2, !5}  ; <2 x %struct.test01*>
!5 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!6 = !{i32 0, i32 0}  ; i32
!7 = distinct !{!8}
!8 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!9 = !{!"S", %struct.test01 zeroinitializer, i32 2, !2, !2} ; { float, float }
!10 = !{!"S", %struct.test02 zeroinitializer, i32 3, !3, !4, !6} ; { i8, <2 x %struct.test01*>, i32 }
