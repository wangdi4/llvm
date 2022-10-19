; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test cases for stores that are marked 'volatile' which store structure field
; elements.
; These cases should trigger the "Volatile data" safety flag on the structure.

; Store to a field member.
%struct.test01 = type { i32, i32, i32 }
@pStruct01 = internal global %struct.test01 zeroinitializer
define void @test01(i32 %value) {
  %pField = getelementptr %struct.test01, %struct.test01* @pStruct01, i64 0, i32 1
  store volatile i32 %value, i32* %pField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Volatile data | Global instance{{ *$}}
; CHECK: End LLVMType: %struct.test01


; Store to a pointer of another type, safety data is not pointer-carried.
%struct.test02a = type { %struct.test02b* }
%struct.test02b = type { i32 }
@pStruct02 = internal global %struct.test02a zeroinitializer
define void @test02(%struct.test02b* "intel_dtrans_func_index"="1" %value) !intel.dtrans.func.type !3 {
  %pField = getelementptr %struct.test02a, %struct.test02a* @pStruct02, i64 0, i32 0
  store volatile %struct.test02b* %value, %struct.test02b** %pField
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


; Use of nested type
%struct.test03a = type { i32, %struct.test03b }
%struct.test03b = type { i32 }
@pStruct03 = internal global %struct.test03a zeroinitializer
define void @test03(i32 %value) {
  %pField = getelementptr %struct.test03a, %struct.test03a* @pStruct03, i64 0, i32 0
  store volatile i32 %value, i32* %pField
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
!3 = distinct !{!2}
!4 = !{%struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!5 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!6 = !{!"S", %struct.test02a zeroinitializer, i32 1, !2} ; { %struct.test02b* }
!7 = !{!"S", %struct.test02b zeroinitializer, i32 1, !1} ; { i32 }
!8 = !{!"S", %struct.test03a zeroinitializer, i32 2, !1, !4} ; { i32, %struct.test03b }
!9 = !{!"S", %struct.test03b zeroinitializer, i32 1, !1} ; { i32 }

!intel.dtrans.types = !{!5, !6, !7, !8, !9}
