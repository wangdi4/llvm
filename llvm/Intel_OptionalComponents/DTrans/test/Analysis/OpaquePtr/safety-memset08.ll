; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s

; Test calls to memset with pointer types.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Test a case where the ValueTypeInfo has an element pointee, and aliases to
; aggregate types because the field is a pointer to a structure type. Here just
; the pointer fields are being written, not the elements of the pointed-to
; structures.
%struct.test01a = type { i64, %struct.test01b*, %struct.test01b*, i64, i64 }
%struct.test01b = type { i32, i32 }
define void @test01(%struct.test01a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !5 {
  %pField = getelementptr %struct.test01a, %struct.test01a* %pStruct, i64 0, i32 1
  %pStart = bitcast %struct.test01b** %pField to i8*
  call void @llvm.memset.p0i8.i64(i8* %pStart, i8 1, i64 16, i1 false)
  ret void
}
; Not including the actual types of the pointer fields in check lines, because
; with opaque pointers they will just be 'ptr'
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: 0)Field LLVM Type: i64
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type:
; CHECK: Field info: Written ComplexUse
; CHECK: 2)Field LLVM Type:
; CHECK: Field info: Written ComplexUse
; CHECK: 3)Field LLVM Type: i64
; CHECK: Field info:{{ *$}}
; CHECK: 4)Field LLVM Type: i64
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Memfunc partial write{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01b


; Test where pointer passed to memset is value loaded from a field, and does not
; impact the structure type at all.
%struct.test02 = type { i32* }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !8 {
  %pField = getelementptr %struct.test02, %struct.test02* %pStruct, i64 0, i32 0
  %pValue = load i32*, i32** %pField
  %pStart = bitcast i32* %pValue to i8*
  call void @llvm.memset.p0i8.i64(i8* %pStart, i8 1, i64 40, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: 0)Field LLVM Type:
; CHECK: Field info: Read{{ *$}}
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test02


; Test case where pointer-to-pointer to structure type is passed to memset.
%struct.test03 = type { i32, i32, i32 }
define "intel_dtrans_func_index"="1" %struct.test03** @test03() !intel.dtrans.func.type !10 {
  %mem = call i8* @malloc(i64 64)
  call void @llvm.memset.p0i8.i64(i8* %mem, i8 0, i64 64, i1 false)
  %structs = bitcast i8* %mem to %struct.test03**
  ret %struct.test03** %structs
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test03


declare !intel.dtrans.func.type !12 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8, i64, i1)
declare !intel.dtrans.func.type !13 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test01b zeroinitializer, i32 1}  ; %struct.test01b*
!3 = !{i32 0, i32 0}  ; i32
!4 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!5 = distinct !{!4}
!6 = !{i32 0, i32 1}  ; i32*
!7 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!8 = distinct !{!7}
!9 = !{%struct.test03 zeroinitializer, i32 2}  ; %struct.test03**
!10 = distinct !{!9}
!11 = !{i8 0, i32 1}  ; i8*
!12 = distinct !{!11}
!13 = distinct !{!11}
!14 = !{!"S", %struct.test01a zeroinitializer, i32 5, !1, !2, !2, !1, !1} ; { i64, %struct.test01b*, %struct.test01b*, i64, i64 }
!15 = !{!"S", %struct.test01b zeroinitializer, i32 2, !3, !3} ; { i32, i32 }
!16 = !{!"S", %struct.test02 zeroinitializer, i32 1, !6} ; { i32* }
!17 = !{!"S", %struct.test03 zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }

!intel.dtrans.types = !{!14, !15, !16, !17}
