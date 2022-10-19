; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test cases where a load uses a pointer to a field in a structure, and the loaded
; type does not correspond to the field type.
; These cases are for when the field type is a pointer to an aggregate type,
; but gets accessed as a scalar type.


%struct.test01a = type { %struct.test01b*, %struct.test01b*, %struct.test01b* }
%struct.test01b = type { i32, i32, i32 }
define void @test01(%struct.test01a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !4 {
  %pField = getelementptr %struct.test01a, %struct.test01a* %pStruct, i64 0, i32 1
  %pField.as.p8 = bitcast %struct.test01b** %pField to i8*
  %vField = load i8, i8* %pField.as.p8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Bad casting
; CHECK: End LLVMType: %struct.test01b


%struct.test02a = type { %struct.test02b*, %struct.test02b*, %struct.test02b* }
%struct.test02b = type { i32, i32, i32 }
define void @test02(%struct.test02a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !7 {
  %pField = getelementptr %struct.test02a, %struct.test02a* %pStruct, i64 0, i32 1
  %pField.as.p16 = bitcast %struct.test02b** %pField to i16*
  %vField = load i16, i16* %pField.as.p16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test02a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02b
; CHECK: Safety data: Bad casting{{ *$}}
; CHECK: End LLVMType: %struct.test02b


; This case does not trigger a "Mismatched element access" because it is using
; a pointer sized int to load a pointer.
%struct.test03a = type { %struct.test03b*, %struct.test03b*, %struct.test03b* }
%struct.test03b = type { i32, i32, i32 }
define void @test03(%struct.test03a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !10 {
  %pField = getelementptr %struct.test03a, %struct.test03a* %pStruct, i64 0, i32 1
  %pField.as.p64 = bitcast %struct.test03b** %pField to i64*
  %vField = load i64, i64* %pField.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03a
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test03a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03b
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test03b


%struct.test04a = type { %struct.test04b** }
%struct.test04b = type { i32, i32, i32 }
define void @test04(%struct.test04a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !13 {
  %pField = getelementptr %struct.test04a, %struct.test04a* %pStruct, i64 0, i32 0
  %pField.as.p16 = bitcast %struct.test04b*** %pField to i16*
  %vField = load i16, i16* %pField.as.p16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test04a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04b
; CHECK: Safety data: Bad casting{{ *$}}
; CHECK: End LLVMType: %struct.test04b


; This case does not trigger a "Mismatched element access" because it is using
; a pointer sized int to load a pointer to a pointer. Note, %vField is still
; being tracked as a %struct.test05b**, so could result in a safety bit later
; based on its uses.
%struct.test05a = type { %struct.test05b**, %struct.test05b*, %struct.test05b* }
%struct.test05b = type { i32, i32, i32 }
define void @test05(%struct.test05a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !17 {
  %pField = getelementptr %struct.test05a, %struct.test05a* %pStruct, i64 0, i32 0
  %pField.as.p64 = bitcast %struct.test05b*** %pField to i64*
  %vField = load i64, i64* %pField.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05a
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test05a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05b
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test05b

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
!11 = !{%struct.test04b zeroinitializer, i32 2}  ; %struct.test04b**
!12 = !{%struct.test04a zeroinitializer, i32 1}  ; %struct.test04a*
!13 = distinct !{!12}
!14 = !{%struct.test05b zeroinitializer, i32 2}  ; %struct.test05b**
!15 = !{%struct.test05b zeroinitializer, i32 1}  ; %struct.test05b*
!16 = !{%struct.test05a zeroinitializer, i32 1}  ; %struct.test05a*
!17 = distinct !{!16}
!18 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { %struct.test01b*, %struct.test01b*, %struct.test01b* }
!19 = !{!"S", %struct.test01b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!20 = !{!"S", %struct.test02a zeroinitializer, i32 3, !5, !5, !5} ; { %struct.test02b*, %struct.test02b*, %struct.test02b* }
!21 = !{!"S", %struct.test02b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!22 = !{!"S", %struct.test03a zeroinitializer, i32 3, !8, !8, !8} ; { %struct.test03b*, %struct.test03b*, %struct.test03b* }
!23 = !{!"S", %struct.test03b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!24 = !{!"S", %struct.test04a zeroinitializer, i32 1, !11} ; { %struct.test04b** }
!25 = !{!"S", %struct.test04b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!26 = !{!"S", %struct.test05a zeroinitializer, i32 3, !14, !15, !15} ; { %struct.test05b**, %struct.test05b*, %struct.test05b* }
!27 = !{!"S", %struct.test05b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }

!intel.dtrans.types = !{!18, !19, !20, !21, !22, !23, !24, !25, !26, !27}
