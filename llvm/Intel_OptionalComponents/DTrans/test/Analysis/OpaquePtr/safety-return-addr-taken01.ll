; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that returning a pointer to a structure type as a generic type
; results in the "Address taken" safety bit.

; Return an i8* typed pointer for a pointer to a structure.
%struct.test01 = type { i32, i32 }
define i8* @test3(%struct.test01* %pStruct) !dtrans_type !2 {
  %pStruct.as.p8 = bitcast %struct.test01* %pStruct to i8*
  ret i8* %pStruct.as.p8
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Address taken{{ *$}}


; Return a pointer-sized int for a pointer to a structure.
%struct.test02 = type { i32, i32 }
define i64 @test4(%struct.test02* %pStruct) !dtrans_type !6 {
  %pStruct.as.i64 = ptrtoint %struct.test02* %pStruct to i64
  ret i64 %pStruct.as.i64
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Address taken{{ *$}}


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !3, !4}  ; i8* (%struct.test01*)
!3 = !{i8 0, i32 1}  ; i8*
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{!"F", i1 false, i32 1, !7, !8}  ; i64 (%struct.test02*)
!7 = !{i64 0, i32 0}  ; i64
!8 = !{!9, i32 1}  ; %struct.test02*
!9 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!10 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!11 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!10, !11}
