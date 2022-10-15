; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases for loads that are marked 'volatile' which load element pointees

; These cases should trigger the "Volatile data" safety flag.


; Test that 'Volatile data' gets marked on the structure for volatile loads
; of elements
%struct.test01 = type { i32, i32, i32 }
@pStruct01 = internal global %struct.test01 zeroinitializer
define void @test01() {
  %pField = getelementptr %struct.test01, %struct.test01* @pStruct01, i64 0, i32 1
  %val = load volatile i32, i32* %pField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Volatile data | Global instance{{ *$}}
; CHECK: End LLVMType: %struct.test01


; Test that 'Volatile data' gets marked on the structure for volatile loads
; of elements, but not the type pointed to.
%struct.test02a = type { %struct.test02b* }
%struct.test02b = type { i32 }
@pStruct02 = internal global %struct.test02a zeroinitializer
define void @test02() {
  %pField = getelementptr %struct.test02a, %struct.test02a* @pStruct02, i64 0, i32 0
  %val = load volatile %struct.test02b*, %struct.test02b** %pField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02a
; CHECK: Safety data: Volatile data | Global instance{{ *$}}
; CHECK: End LLVMType: %struct.test02a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02b
; CHECK: Safety data:
; CHECK-SAME: No issues found
; CHECK: End LLVMType: %struct.test02b


; Volatile data will be cascaded to nested types when one field is loaded with
; the volatile specifier.
%struct.test03a = type { i32, %struct.test03b }
%struct.test03b = type { i32 }
@pStruct03 = internal global %struct.test03a zeroinitializer
define void @test03() {
  %pField = getelementptr %struct.test03a, %struct.test03a* @pStruct03, i64 0, i32 0
  %val = load volatile i32, i32* %pField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03a
; CHECK: Safety data: Volatile data | Global instance | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test03a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03b
; CHECK: Safety data: Volatile data | Global instance | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test03b


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test02b zeroinitializer, i32 1}  ; %struct.test02b*
!3 = !{%struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!4 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!5 = !{!"S", %struct.test02a zeroinitializer, i32 1, !2} ; { %struct.test02b* }
!6 = !{!"S", %struct.test02b zeroinitializer, i32 1, !1} ; { i32 }
!7 = !{!"S", %struct.test03a zeroinitializer, i32 2, !1, !3} ; { i32, %struct.test03b }
!8 = !{!"S", %struct.test03b zeroinitializer, i32 1, !1} ; { i32 }

!intel.dtrans.types = !{!4, !5, !6, !7, !8}
