; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s

; Test calls to memset with pointer types.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Test a case where the ValueTypeInfo has an element pointee, and aliases to
; aggregate types because the field is a pointer to a structure type. Here just
; the pointer fields are being written, not the elements of the pointed-to
; structures.
%struct.test01a = type { i64, %struct.test01b*, %struct.test01b*, i64, i64 }
%struct.test01b = type { i32, i32 }
define void @test01(%struct.test01a* %pStruct) !dtrans_type !5 {
  %pField = getelementptr %struct.test01a, %struct.test01a* %pStruct, i64 0, i32 1
  %pStart = bitcast %struct.test01b** %pField to i8*
  call void @llvm.memset.p0i8.i64(i8* %pStart, i8 1, i64 16, i1 false)
  ret void
}
; Not including the actual types of the pointer fields in check lines, because
; with opaque pointers they will just be 'p0'
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: 0)Field LLVM Type: i64
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type:
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type:
; CHECK: Field info: Written
; CHECK: 3)Field LLVM Type: i64
; CHECK: Field info:{{ *$}}
; CHECK: 4)Field LLVM Type: i64
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Memfunc partial write{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: No issues found


; Test where pointer passed to memset is value loaded from a field, and does not
; impact the structure type at all.
%struct.test02 = type { i32* }
define void @test02(%struct.test02* %pStruct) !dtrans_type !10 {
  %pField = getelementptr %struct.test02, %struct.test02* %pStruct, i64 0, i32 0
  %pValue = load i32*, i32** %pField
  %pStart = bitcast i32* %pValue to i8*
  call void @llvm.memset.p0i8.i64(i8* %pStart, i8 1, i64 40, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: 0)Field LLVM Type:
; CHECK: Field info: Read{{ *$}}
; CHECK: Safety data: No issues found


; Test case where pointer-to-pointer to structure type is passed to memset.
%struct.test03 = type { i32, i32, i32 }
define %struct.test03** @test03() !dtrans_type !13 {
  %mem = call i8* @malloc(i64 64)
  call void @llvm.memset.p0i8.i64(i8* %mem, i8 0, i64 64, i1 false)
  %structs = bitcast i8* %mem to %struct.test03**
  ret %struct.test03** %structs
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: No issues found


declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
declare i8* @malloc(i64)

!1 = !{i64 0, i32 0}  ; i64
!2 = !{!3, i32 1}  ; %struct.test01b*
!3 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!4 = !{i32 0, i32 0}  ; i32
!5 = !{!"F", i1 false, i32 1, !6, !7}  ; void (%struct.test01a*)
!6 = !{!"void", i32 0}  ; void
!7 = !{!8, i32 1}  ; %struct.test01a*
!8 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!9 = !{i32 0, i32 1}  ; i32*
!10 = !{!"F", i1 false, i32 1, !6, !11}  ; void (%struct.test02*)
!11 = !{!12, i32 1}  ; %struct.test02*
!12 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!13 = !{!"F", i1 false, i32 0, !14}  ; %struct.test03** ()
!14 = !{!15, i32 2}  ; %struct.test03**
!15 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!16 = !{!"S", %struct.test01a zeroinitializer, i32 5, !1, !2, !2, !1, !1} ; { i64, %struct.test01b*, %struct.test01b*, i64, i64 }
!17 = !{!"S", %struct.test01b zeroinitializer, i32 2, !4, !4} ; { i32, i32 }
!18 = !{!"S", %struct.test02 zeroinitializer, i32 1, !9} ; { i32* }
!19 = !{!"S", %struct.test03 zeroinitializer, i32 3, !4, !4, !4} ; { i32, i32, i32 }

!dtrans_types = !{!16, !17, !18, !19}
