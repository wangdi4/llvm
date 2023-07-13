; Test case where a small structure gets treated as a vector type to verify that
; an element pointee collected by the PtrTypeAnalyzer does not lead to an
; assertion in the DTransSafetyAnalyzer. (CMPLRLLVM-46986)

; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

%struct.test01 = type { float, float }
%struct.test02 = type { i8, %struct.test01, i32 }

define float @test(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !7 {
  %a = getelementptr %struct.test02, ptr %in, i64 0, i32 1

  ; This instruction should trigger a safety condition on
  ; %struct.test01, becuase %a is expected to be used as the
  ; type %struct.test01* based on the structure definition
  ; of %struct.test02, but is instead used as a vector type,
  ; even though the memory layout matches.
  %b = getelementptr <2 x float>, ptr %a, i64 0, i32 1
  
  ; The type for the pointer being loaded was collected as the element pointee:
  ;   <2 x float> @ 1
  ; This needs to be handled when the DTransSafetyAnalyzer is analyzing
  ; element loads/stores.
  %c = load float, ptr %b
  ret float %c
}

; For DTrans, we want the structure, %struct.test01, marked with a safety flag
; that prevents changes to the structure because we omit collecting field usage
; information for such a case, and transformations do not support transforming
; the field accesses if the structure fields are changed.

; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Ambiguous GEP | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01

; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02

!1 = !{float 0.0e+00, i32 0}  ; float
!2 = !{i8 0, i32 0}  ; i8
!3 = !{%struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!4 = !{i32 0, i32 0}  ; i32
!5 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { float, float }
!6 = !{!"S", %struct.test02 zeroinitializer, i32 3, !2, !3, !4} ; { i8, %struct.test01, i32 }
!7 = distinct !{!8}
!8 = !{%struct.test02 zeroinitializer, i32 1} ; %struct.test02*

!intel.dtrans.types = !{!5, !6}
