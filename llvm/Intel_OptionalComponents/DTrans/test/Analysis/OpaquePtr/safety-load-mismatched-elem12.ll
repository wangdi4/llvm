; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test cases where a load uses a pointer to the start of a structure, but
; loads a type that does not match the type of the first element of the
; structure.

; These cases are for when the field element is a pointer to a structure type,
; and the load type is a pointer of a different type.

; This case does not trigger a "Mismatched element access" because it is using
; a i8* to load a pointer to the structure pointer, which is safe.
%struct.test01a = type { %struct.test01b*, %struct.test01b*, %struct.test01b* }
%struct.test01b = type { i32, i32, i32 }
define void @test01(%struct.test01a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !4 {
  %pStruct.as.pp8 = bitcast %struct.test01a* %pStruct to i8**
  %vField = load i8*, i8** %pStruct.as.pp8

  ; This instruction is needed for the pointer type analyzer to identify
  ; %vField as being used as an i8* type.
  call void @free(i8* %vField)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01b


%struct.test02a = type{ %struct.test02b*, %struct.test02b*, %struct.test02b* }
%struct.test02b = type { i32, i32, i32 }
define void @test02(%struct.test02a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !7 {
  %pStruct.as.pp16 = bitcast %struct.test02a* %pStruct to i16**
  %vField = load i16*, i16** %pStruct.as.pp16

  ; This instruction is needed for the pointer type analyzer to identify
  ; %vField as being used as an i16* type.
  %use = getelementptr i16, i16* %vField, i64 4
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test02a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02b
; CHECK: Safety data: Bad casting | Ambiguous GEP | Unhandled use{{ *$}}
; CHECK: End LLVMType: %struct.test02b


%struct.test03a = type{ %struct.test03b*, %struct.test03b*, %struct.test03b* }
%struct.test03b = type { i32, i32, i32 }
define void @test03(%struct.test03a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !10 {
  %pStruct.as.pp64 = bitcast %struct.test03a* %pStruct to i64**
  %vField = load i64*, i64** %pStruct.as.pp64

  ; Use vField as an i64* type so that the local pointer analyzer will mark
  ; it 'i64*' as one of the aliases, because otherwise there is no use seen
  ; as being an i64*.
  %use = load i64, i64* %vField

  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03a
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test03a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03b
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test03b

declare !intel.dtrans.func.type !12 void @free(i8* "intel_dtrans_func_index"="1") #0

attributes #0 = { allockind("free") "alloc-family"="malloc" }

!1 = !{%struct.test01b zeroinitializer, i32 1}  ; %struct.test01b*
!2 = !{i32 0, i32 0}  ; i32
!3 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!4 = distinct !{!3}
!5 = !{%struct.test02b zeroinitializer, i32 1}  ; %struct.test02b*
!6 = !{%struct.test02a zeroinitializer, i32 1}  ; %struct.test02a*
!7 = distinct !{!6}
!8 = !{%struct.test03b zeroinitializer, i32 1}  ; %struct.test03b*
!9 = !{%struct.test03a zeroinitializer, i32 1}  ; %struct.test03a*
!10 = distinct !{!9}
!11 = !{i8 0, i32 1}  ; i8*
!12 = distinct !{!11}
!13 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { %struct.test01b*, %struct.test01b*, %struct.test01b* }
!14 = !{!"S", %struct.test01b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!15 = !{!"S", %struct.test02a zeroinitializer, i32 3, !5, !5, !5} ; { %struct.test02b*, %struct.test02b*, %struct.test02b* }
!16 = !{!"S", %struct.test02b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!17 = !{!"S", %struct.test03a zeroinitializer, i32 3, !8, !8, !8} ; { %struct.test03b*, %struct.test03b*, %struct.test03b* }
!18 = !{!"S", %struct.test03b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }

!intel.dtrans.types = !{!13, !14, !15, !16, !17, !18}
