; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test cases where a load uses a pointer to the start of a structure, but
; loads a type that does not match the type of the first element of the
; structure.

; These cases are for when the field element is a scalar type, and the load
; type is a pointer type.

%struct.test01 = type { i32, i32, i32 }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !3 {
  %pStruct.as.pp8 = bitcast %struct.test01* %pStruct to i8**
  %vField = load i8*, i8** %pStruct.as.pp8

  ; This instruction is needed for the pointer type analyzer to identify
  ; %vField as being used as an i8* type.
  %use = load i8, i8* %vField
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test01


%struct.test02 = type { i32, i32, i32 }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !5 {
  %pStruct.as.pp32 = bitcast %struct.test02* %pStruct to i32**
  %vField = load i32*, i32** %pStruct.as.pp32

  ; This instruction is needed for the pointer type analyzer to identify
  ; %vField as being used as a i32* type.
  %use = load i32, i32* %vField
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test02


%struct.test03 = type { i32, i32, i32 }
define void @test03(%struct.test03* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !7 {
  %pStruct.as.pp64 = bitcast %struct.test03* %pStruct to i64**
  %vField = load i64*, i64** %pStruct.as.pp64

  ; This instruction is needed for the pointer type analyzer to identify
  ; %vField as being used as a i64* type.
  %use = load i64, i64* %vField
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test03


%struct.test04a = type { i32, i32, i32 }
%struct.test04b = type { i32 }
define void @test04(%struct.test04a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !9 {
  %pStruct.as.ppS4b = bitcast %struct.test04a* %pStruct to %struct.test04b**
  %vField = load %struct.test04b*, %struct.test04b** %pStruct.as.ppS4b
  %use_test04 = getelementptr %struct.test04b, %struct.test04b* %vField, i64 0, i32 0
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test04a

; No elements of %struct.test04b are being accessed, so there should not be
; 'Mismatched element access' on it.
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04b
; CHECK: Safety data: Bad casting{{ *$}}
; CHECK: End LLVMType: %struct.test04b


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4}
!6 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!7 = distinct !{!6}
!8 = !{%struct.test04a zeroinitializer, i32 1}  ; %struct.test04a*
!9 = distinct !{!8}
!10 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!11 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!12 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!13 = !{!"S", %struct.test04a zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!14 = !{!"S", %struct.test04b zeroinitializer, i32 1, !1} ; { i32 }

!intel.dtrans.types = !{!10, !11, !12, !13, !14}
