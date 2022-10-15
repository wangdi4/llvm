; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases of load instructions that should be identified as "safe"
; These cases cover the "safe" forms of the tests in
; safety-load-mismatched-elem[01-13].ll

%struct.test01 = type { i32, i32, i32 }
define i32 @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !3 {
  %pField = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 1
  %vField = load i32, i32* %pField
  ret i32 %vField
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Read{{ *$}}
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01

%struct.test02 = type { i32*, i32*, i32* }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !6 {
  %pField = getelementptr %struct.test02, %struct.test02* %pStruct, i64 0, i32 1
  %vField = load i32*, i32** %pField
  %use = load i32, i32* %vField
  ret void
}
; TODO: Field types will be 'p0' when opaque pointers are enabled
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: 0)Field LLVM Type: i32*
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32*
; CHECK: Field info: Read UnusedValue{{ *$}}
; CHECK: 2)Field LLVM Type: i32*
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test02


%struct.test03a = type { %struct.test03b*, %struct.test03b*, %struct.test03b* }
%struct.test03b = type { i32, i32, i32 }
define i32 @test03(%struct.test03a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !9 {
  %pField = getelementptr %struct.test03a, %struct.test03a* %pStruct, i64 0, i32 1
  %vField = load %struct.test03b*, %struct.test03b** %pField
  %pFieldB = getelementptr %struct.test03b, %struct.test03b* %vField, i64 0, i32 2
  %vFieldB = load i32, i32* %pFieldB
  ret i32 %vFieldB
}
; TODO: Field types will be 'p0' when opaque pointers are enabled
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03a
; CHECK: 0)Field LLVM Type: %struct.test03b*
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: %struct.test03b*
; CHECK: Field info: Read{{ *$}}
; CHECK: 2)Field LLVM Type: %struct.test03b*
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test03a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03b
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Read{{ *$}}
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test03b


; Load element zero using a pointer to the structure
%struct.test04 = type { i32, i32 }
define void @test04(%struct.test04* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !11 {
  %pStruct.as.p32 = bitcast %struct.test04* %pStruct to i32*
  %vField = load i32, i32* %pStruct.as.p32
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Read UnusedValue NonGEPAccess{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test04

; Load element zero of a nested structure using a pointer to the structure
%struct.test05a = type { %struct.test05b }
%struct.test05b = type { i32, i32 }
define i32 @test05(%struct.test05a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !14 {
  %pField = getelementptr %struct.test05a, %struct.test05a* %pStruct, i64 0, i32 0
  %pField.as.p32 = bitcast %struct.test05b* %pField to i32*
  %vField = load i32, i32* %pField.as.p32
  ret i32 %vField
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05a
; CHECK: 0)Field LLVM Type: %struct.test05b
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test05a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05b
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Read NonGEPAccess{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test05b


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{i32 0, i32 1}  ; i32*
!5 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!6 = distinct !{!5}
!7 = !{%struct.test03b zeroinitializer, i32 1}  ; %struct.test03b*
!8 = !{%struct.test03a zeroinitializer, i32 1}  ; %struct.test03a*
!9 = distinct !{!8}
!10 = !{%struct.test04 zeroinitializer, i32 1}  ; %struct.test04*
!11 = distinct !{!10}
!12 = !{%struct.test05b zeroinitializer, i32 0}  ; %struct.test05b
!13 = !{%struct.test05a zeroinitializer, i32 1}  ; %struct.test05a*
!14 = distinct !{!13}
!15 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!16 = !{!"S", %struct.test02 zeroinitializer, i32 3, !4, !4, !4} ; { i32*, i32*, i32* }
!17 = !{!"S", %struct.test03a zeroinitializer, i32 3, !7, !7, !7} ; { %struct.test03b*, %struct.test03b*, %struct.test03b* }
!18 = !{!"S", %struct.test03b zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!19 = !{!"S", %struct.test04 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!20 = !{!"S", %struct.test05a zeroinitializer, i32 1, !12} ; { %struct.test05b }
!21 = !{!"S", %struct.test05b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!15, !16, !17, !18, !19, !20, !21}
