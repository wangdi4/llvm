; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a load uses a pointer to a field in a structure, and the loaded
; type does not correspond to the field type.
; These cases are for when the field type is a pointer to an aggregate type,
; but gets accessed as a pointer to a different type.

; This case does not trigger a "Mismatched element access" because it is using a
; i8* to load a pointer to the structure pointer.
%struct.test01a = type { ptr, ptr, ptr }
%struct.test01b = type { i32, i32, i32 }
define void @test01(ptr "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !4 {
  %pField = getelementptr %struct.test01a, ptr %pStruct, i64 0, i32 1
  %vField = load ptr, ptr %pField

  ; This instruction is needed for the pointer type analyzer to identify
  ; %vField as being used as an i8* type.
  %use = getelementptr i8, ptr %vField, i64 8
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


%struct.test02a = type{ ptr, ptr, ptr }
%struct.test02b = type { i32, i32, i32 }
define void @test02(ptr "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !7 {
  %pField = getelementptr %struct.test02a, ptr %pStruct, i64 0, i32 1
  %vField = load ptr, ptr %pField

  ; This instruction is needed for the pointer type analyzer to identify
  ; %vField as being used as an i16* type. This instruction causes %test.test02b
  ; to be marked as an "Ambiguous GEP" because %vField is believed to be a
  ; pointer to a structure type. %test.test02b also gets marked as 'Unhandled use'
  ; because the GEP is indexing a field without a structure type or byte-flattened
  ; form.
  %use = getelementptr i16, ptr %vField, i64 4
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


%struct.test03a = type{ ptr, ptr, ptr }
%struct.test03b = type { i32, i32, i32 }
define void @test03(ptr "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !10 {
  %pField = getelementptr %struct.test03a, ptr %pStruct, i64 0, i32 1
  %vField = load ptr, ptr %pField

  ; Use %vField as an i64* type so that the local pointer analyzer will mark
  ; it 'i64*' as one of the aliases, because otherwise there it no use seen
  ; as being an i64*. In this case, the first load of the pointer from test03a
  ; is safe, however, when using that value as an i64 will be an invalid access
  ; for loading an element of test03b.
  %v64 = load i64, ptr %vField
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


%struct.test04a = type { ptr, ptr, ptr }
%struct.test04b = type { i32, i32, i32 }
%struct.test04c = type { i16, i16 }
define void @test04(ptr "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !14 {
  %pField = getelementptr %struct.test04a, ptr %pStruct, i64 0, i32 1
  %vField = load ptr, ptr %pField
  %use4b = getelementptr %struct.test04c, ptr %vField, i64 0, i32 1
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test04a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04b
; CHECK: Safety data: Bad casting | Ambiguous GEP{{ *$}}
; CHECK: End LLVMType: %struct.test04b

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04c
; CHECK: Safety data: Bad casting | Ambiguous GEP{{ *$}}
; CHECK: End LLVMType: %struct.test04c


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
!11 = !{%struct.test04b zeroinitializer, i32 1}  ; %struct.test04b*
!12 = !{i16 0, i32 0}  ; i16
!13 = !{%struct.test04a zeroinitializer, i32 1}  ; %struct.test04a*
!14 = distinct !{!13}
!15 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { %struct.test01b*, %struct.test01b*, %struct.test01b* }
!16 = !{!"S", %struct.test01b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!17 = !{!"S", %struct.test02a zeroinitializer, i32 3, !5, !5, !5} ; { %struct.test02b*, %struct.test02b*, %struct.test02b* }
!18 = !{!"S", %struct.test02b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!19 = !{!"S", %struct.test03a zeroinitializer, i32 3, !8, !8, !8} ; { %struct.test03b*, %struct.test03b*, %struct.test03b* }
!20 = !{!"S", %struct.test03b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!21 = !{!"S", %struct.test04a zeroinitializer, i32 3, !11, !11, !11} ; { %struct.test04b*, %struct.test04b*, %struct.test04b* }
!22 = !{!"S", %struct.test04b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!23 = !{!"S", %struct.test04c zeroinitializer, i32 2, !12, !12} ; { i16, i16 }

!intel.dtrans.types = !{!15, !16, !17, !18, !19, !20, !21, !22, !23}
