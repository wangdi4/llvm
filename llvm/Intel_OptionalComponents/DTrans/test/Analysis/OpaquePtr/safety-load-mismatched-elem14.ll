; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test for loading element zero of a nested type that resolves to an array
; element when using an incorrect type for the elements in the array.

%struct.test01a = type { i64, %struct.test01b }
%struct.test01b = type { %struct.test01c }
%struct.test01c = type { [8 x i32], i64 }
define i64 @test01(%struct.test01a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !7 {
  %pField = getelementptr %struct.test01a, %struct.test01a* %pStruct, i64 0, i32 1
  %pField.as.i64 = bitcast %struct.test01b* %pField to i64*
  %val = load i64, i64* %pField.as.i64
  ret i64 %val
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Mismatched element access | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Mismatched element access | Nested structure | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01b

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01c
; CHECK: Safety data: Mismatched element access | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01c


!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!3 = !{%struct.test01c zeroinitializer, i32 0}  ; %struct.test01c
!4 = !{!"A", i32 8, !5}  ; [8 x i32]
!5 = !{i32 0, i32 0}  ; i32
!6 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !2} ; { i64, %struct.test01b }
!9 = !{!"S", %struct.test01b zeroinitializer, i32 1, !3} ; { %struct.test01c }
!10 = !{!"S", %struct.test01c zeroinitializer, i32 2, !4, !1} ; { [8 x i32], i64 }

!intel.dtrans.types = !{!8, !9, !10}
