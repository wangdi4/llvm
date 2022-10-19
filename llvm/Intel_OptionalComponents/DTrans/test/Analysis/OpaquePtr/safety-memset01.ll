; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test the safety analysis for calls to memset that take the address of an
; element within a structure type.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Write the entire structure, starting from a GEP of field 0.
%struct.test01 = type { i32, i32, i32, i32, i32 }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !3 {
  %pField = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 0
  %pStart = bitcast i32* %pField to i8*
  call void @llvm.memset.p0i8.i64(i8* %pStart, i8 1, i64 20, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 3)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 4)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01


; Write a subset of structure fields, starting from a GEP of field 0.
%struct.test02 = type { i32, i32, i32, i32, i32 }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !5 {
  %pField = getelementptr %struct.test02, %struct.test02* %pStruct, i64 0, i32 0
  %pStart = bitcast i32* %pField to i8*
  call void @llvm.memset.p0i8.i64(i8* %pStart, i8 1, i64 12, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written ComplexUse
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written ComplexUse
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Written ComplexUse
; CHECK: 3)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 4)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Memfunc partial write{{ *$}}
; CHECK: End LLVMType: %struct.test02


; Write beyond the end of the structure, starting from a GEP of field 0.
%struct.test03 = type { i32, i32, i32, i32, i32 }
define void @test03(%struct.test03* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !7 {
  %pField = getelementptr %struct.test03, %struct.test03* %pStruct, i64 0, i32 0
  %pStart = bitcast i32* %pField to i8*
  call void @llvm.memset.p0i8.i64(i8* %pStart, i8 1, i64 40, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: Bad memfunc size{{ *$}}
; CHECK: End LLVMType: %struct.test03


; Write a subset of the structure, starting and ending on a field boundary.
%struct.test04 = type { i32, i32, i32, i32, i32 }
define void @test04(%struct.test04* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !9 {
  %pField = getelementptr %struct.test04, %struct.test04* %pStruct, i64 0, i32 2
  %pStart = bitcast i32* %pField to i8*
  call void @llvm.memset.p0i8.i64(i8* %pStart, i8 1, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Written ComplexUse
; CHECK: 3)Field LLVM Type: i32
; CHECK: Field info: Written ComplexUse
; CHECK: 4)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Memfunc partial write{{ *$}}
; CHECK: End LLVMType: %struct.test04


; Write a subset of the structure, with a size that extends beyond the structure end.
%struct.test05 = type { i32, i32, i32, i32, i32 }
define void @test05(%struct.test05* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !11 {
  %pField = getelementptr %struct.test05, %struct.test05* %pStruct, i64 0, i32 2
  %pStart = bitcast i32* %pField to i8*
  call void @llvm.memset.p0i8.i64(i8* %pStart, i8 1, i64 16, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05
; CHECK: Safety data: Bad memfunc size{{ *$}}
; CHECK: End LLVMType: %struct.test05


; Write a subset of the structure, with a size that does not end on a field boundary.
%struct.test06 = type { i32, i32, i32, i32, i32 }
define void @test06(%struct.test06* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !13 {
  %pField = getelementptr %struct.test06, %struct.test06* %pStruct, i64 0, i32 2
  %pStart = bitcast i32* %pField to i8*
  call void @llvm.memset.p0i8.i64(i8* %pStart, i8 1, i64 6, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test06
; CHECK: Safety data: Bad memfunc size{{ *$}}
; CHECK: End LLVMType: %struct.test06


declare !intel.dtrans.func.type !15 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8, i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4}
!6 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!7 = distinct !{!6}
!8 = !{%struct.test04 zeroinitializer, i32 1}  ; %struct.test04*
!9 = distinct !{!8}
!10 = !{%struct.test05 zeroinitializer, i32 1}  ; %struct.test05*
!11 = distinct !{!10}
!12 = !{%struct.test06 zeroinitializer, i32 1}  ; %struct.test06*
!13 = distinct !{!12}
!14 = !{i8 0, i32 1}  ; i8*
!15 = distinct !{!14}
!16 = !{!"S", %struct.test01 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!17 = !{!"S", %struct.test02 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!18 = !{!"S", %struct.test03 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!19 = !{!"S", %struct.test04 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!20 = !{!"S", %struct.test05 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!21 = !{!"S", %struct.test06 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }

!intel.dtrans.types = !{!16, !17, !18, !19, !20, !21}
