; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test calls to memcpy involving  pointer fields and pointer-to-pointer types.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Test a case where the ValueTypeInfo has an element pointee, and aliases to
; aggregate types because the field is a pointer to a structure type. Here just
; the pointer fields are being written, not the elements of the pointed-to
; structures.
%struct.test01a = type { i64, %struct.test01b*, %struct.test01b*, i64, i64 }
%struct.test01b = type { i32, i32 }
define void @test01(%struct.test01a* %pStructA, %struct.test01a* %pStructB) !dtrans_type !5 {
  %pFieldA = getelementptr %struct.test01a, %struct.test01a* %pStructA, i64 0, i32 1
  %pFieldB = getelementptr %struct.test01a, %struct.test01a* %pStructB, i64 0, i32 1
  %pDst = bitcast %struct.test01b** %pFieldA to i8*
  %pSrc = bitcast %struct.test01b** %pFieldB to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 16, i1 false)
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


; Test where pointer passed to memcpy is value loaded from a field, and does not
; impact the structure type at all.
%struct.test02 = type { i32* }
define void @test02(%struct.test02* %pStructA, %struct.test02* %pStructB) !dtrans_type !10 {
  %pFieldA = getelementptr %struct.test02, %struct.test02* %pStructA, i64 0, i32 0
  %pFieldB = getelementptr %struct.test02, %struct.test02* %pStructB, i64 0, i32 0
  %pValueA = load i32*, i32** %pFieldA
  %pValueB = load i32*, i32** %pFieldB
  %pDst = bitcast i32* %pValueA to i8*
  %pSrc = bitcast i32* %pValueB to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 12, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: 0)Field LLVM Type:
; CHECK: Field info: Read{{ *$}}
; CHECK: Safety data: No issues found


; Test case where pointer-to-pointer to structure type is passed to memcpy.
%struct.test03 = type { i32, i32, i32 }
define %struct.test03** @test03(%struct.test03** %pOrig) !dtrans_type !13 {
  %mem = call i8* @malloc(i64 64)
  %pSrc = bitcast %struct.test03** %pOrig to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %mem, i8* %pSrc, i64 8, i1 false)
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


declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)
declare i8* @malloc(i64)

!1 = !{i64 0, i32 0}  ; i64
!2 = !{!3, i32 1}  ; %struct.test01b*
!3 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!4 = !{i32 0, i32 0}  ; i32
!5 = !{!"F", i1 false, i32 2, !6, !7, !7}  ; void (%struct.test01a*, %struct.test01a*)
!6 = !{!"void", i32 0}  ; void
!7 = !{!8, i32 1}  ; %struct.test01a*
!8 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!9 = !{i32 0, i32 1}  ; i32*
!10 = !{!"F", i1 false, i32 2, !6, !11, !11}  ; void (%struct.test02*, %struct.test02*)
!11 = !{!12, i32 1}  ; %struct.test02*
!12 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!13 = !{!"F", i1 false, i32 1, !14, !14}  ; %struct.test03** (%struct.test03**)
!14 = !{!15, i32 2}  ; %struct.test03**
!15 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!16 = !{!"F", i1 false, i32 4, !6, !17, !17, !1, !18}  ; void (i8*, i8*, i64, i1)
!17 = !{i8 0, i32 1}  ; i8*
!18 = !{i1 0, i32 0}  ; i1
!19 = !{!"F", i1 false, i32 1, !17, !1}  ; i8* (i64)
!20 = !{!"S", %struct.test01a zeroinitializer, i32 5, !1, !2, !2, !1, !1} ; { i64, %struct.test01b*, %struct.test01b*, i64, i64 }
!21 = !{!"S", %struct.test01b zeroinitializer, i32 2, !4, !4} ; { i32, i32 }
!22 = !{!"S", %struct.test02 zeroinitializer, i32 1, !9} ; { i32* }
!23 = !{!"S", %struct.test03 zeroinitializer, i32 3, !4, !4, !4} ; { i32, i32, i32 }
!24 = !{!"llvm.memcpy.p0i8.p0i8.i64", !16}
!25 = !{!"malloc", !19}

!dtrans_types = !{!20, !21, !22, !23}
!dtrans_decl_types = !{!24, !25}
