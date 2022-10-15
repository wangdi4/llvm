; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a load uses a pointer to the start of a structure, but
; loads a type that does not match the type of the first element of the
; structure.

; These cases are for when the field element is a nested a structure type,
; and the load type does not match the first element of the nested type.
; These cases also get marked as 'Bad casting' when they are unsafe because
; it means a pointer to the structure is being used as an incorrect type.
%struct.test01a = type { %struct.test01b }
%struct.test01b = type { i32, i32, i32 }
define void @test01(%struct.test01a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !4 {
  %pStruct.as.p8 = bitcast %struct.test01a* %pStruct to i8*
  %vField = load i8, i8* %pStruct.as.p8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Bad casting | Mismatched element access | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; Safety data: Bad casting | Mismatched element access | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01b


%struct.test02a = type { %struct.test02b }
%struct.test02b = type { i32, i32, i32 }
define void @test02(%struct.test02a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !7 {
  %pStruct.as.p16 = bitcast %struct.test02a* %pStruct to i16*
  %vField = load i16, i16* %pStruct.as.p16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02a
; CHECK: Safety data: Bad casting | Mismatched element access | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02b
; CHECK: Safety data: Bad casting | Mismatched element access | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02b


%struct.test03a = type { %struct.test03b }
%struct.test03b = type { i32, i32, i32 }
define void @test03(%struct.test03a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !10 {
  %pStruct.as.p64 = bitcast %struct.test03a* %pStruct to i64*
  %vField = load i64, i64* %pStruct.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03a
; CHECK: Safety data: Bad casting | Mismatched element access | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test03a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03b
; CHECK: Safety data: Bad casting | Mismatched element access | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test03b


; Access that could be considered a whole structure reference, but we will
; not handle that in DTrans, so it should be marked as "mismatched element
; access".
%struct.test04a = type { %struct.test04b }
%struct.test04b = type { i32, i32 }
define void @test04(%struct.test04a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !13 {
  %pStruct.as.p64 = bitcast %struct.test04a* %pStruct to i64*
  %vField = load i64, i64* %pStruct.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04a
; CHECK: Safety data: Bad casting | Mismatched element access | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test04a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04b
; CHECK: Safety data: Bad casting | Mismatched element access | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test04b


%struct.test05a = type { %struct.test05b }
%struct.test05b = type { i32, i32 }
%struct.test05c = type { i64 }
define void @test05(%struct.test05a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !17 {
  %pStruct.as.ppS5c = bitcast %struct.test05a* %pStruct to %struct.test05c**
  %vField = load %struct.test05c*, %struct.test05c** %pStruct.as.ppS5c
  %use = getelementptr %struct.test05c, %struct.test05c* %vField, i64 0, i32 0
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05a
; CHECK: Safety data: Bad casting | Mismatched element access | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test05a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05b
; CHECK: Safety data: Bad casting | Mismatched element access | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test05b

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05c
; CHECK: Safety data: Bad casting{{ *$}}
; CHECK: End LLVMType: %struct.test05c


; A safe access to a nested element.
%struct.test06a = type{ %struct.test06b }
%struct.test06b = type { i32, i32, i32 }
define void @test06(%struct.test06a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !20 {
  %pStruct.as.p32 = bitcast %struct.test06a* %pStruct to i32*
  %vField = load i32, i32* %pStruct.as.p32
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test06a
; CHECK: Safety data: Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test06a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test06b
; CHECK: Safety data: Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test06b



!1 = !{%struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!2 = !{i32 0, i32 0}  ; i32
!3 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!4 = distinct !{!3}
!5 = !{%struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!6 = !{%struct.test02a zeroinitializer, i32 1}  ; %struct.test02a*
!7 = distinct !{!6}
!8 = !{%struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!9 = !{%struct.test03a zeroinitializer, i32 1}  ; %struct.test03a*
!10 = distinct !{!9}
!11 = !{%struct.test04b zeroinitializer, i32 0}  ; %struct.test04b
!12 = !{%struct.test04a zeroinitializer, i32 1}  ; %struct.test04a*
!13 = distinct !{!12}
!14 = !{%struct.test05b zeroinitializer, i32 0}  ; %struct.test05b
!15 = !{i64 0, i32 0}  ; i64
!16 = !{%struct.test05a zeroinitializer, i32 1}  ; %struct.test05a*
!17 = distinct !{!16}
!18 = !{%struct.test06b zeroinitializer, i32 0}  ; %struct.test06b
!19 = !{%struct.test06a zeroinitializer, i32 1}  ; %struct.test06a*
!20 = distinct !{!19}
!21 = !{!"S", %struct.test01a zeroinitializer, i32 1, !1} ; { %struct.test01b }
!22 = !{!"S", %struct.test01b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!23 = !{!"S", %struct.test02a zeroinitializer, i32 1, !5} ; { %struct.test02b }
!24 = !{!"S", %struct.test02b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!25 = !{!"S", %struct.test03a zeroinitializer, i32 1, !8} ; { %struct.test03b }
!26 = !{!"S", %struct.test03b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!27 = !{!"S", %struct.test04a zeroinitializer, i32 1, !11} ; { %struct.test04b }
!28 = !{!"S", %struct.test04b zeroinitializer, i32 2, !2, !2} ; { i32, i32 }
!29 = !{!"S", %struct.test05a zeroinitializer, i32 1, !14} ; { %struct.test05b }
!30 = !{!"S", %struct.test05b zeroinitializer, i32 2, !2, !2} ; { i32, i32 }
!31 = !{!"S", %struct.test05c zeroinitializer, i32 1, !15} ; { i64 }
!32 = !{!"S", %struct.test06a zeroinitializer, i32 1, !18} ; { %struct.test06b }
!33 = !{!"S", %struct.test06b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }

!intel.dtrans.types = !{!21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33}
