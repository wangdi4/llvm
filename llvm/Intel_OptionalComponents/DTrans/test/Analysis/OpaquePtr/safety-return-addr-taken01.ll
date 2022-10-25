; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that returning a pointer to a structure type as a generic type
; results in the "Address taken" safety bit.

; Return an i8* typed pointer for a pointer to a structure.
%struct.test01 = type { i32, i32 }
define "intel_dtrans_func_index"="1" i8* @test3(%struct.test01* "intel_dtrans_func_index"="2" %pStruct) !intel.dtrans.func.type !4 {
  %pStruct.as.p8 = bitcast %struct.test01* %pStruct to i8*
  ret i8* %pStruct.as.p8
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Address taken{{ *$}}
; CHECK: End LLVMType: %struct.test01


; Return a pointer-sized int for a pointer to a structure.
%struct.test02 = type { i32, i32 }
define i64 @test4(%struct.test02* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !6 {
  %pStruct.as.i64 = ptrtoint %struct.test02* %pStruct to i64
  ret i64 %pStruct.as.i64
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Address taken{{ *$}}
; CHECK: End LLVMType: %struct.test02


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i8 0, i32 1}  ; i8*
!3 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!4 = distinct !{!2, !3}
!5 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!6 = distinct !{!5}
!7 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!8 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!7, !8}
