; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases for safety analysis of memmove calls. The logic for calls to
; memmove is the same as memcpy, so this test just does sanity checking that
; memmove is recognized. For more complete testing of safety conditions that may
; impact memmove calls, refer to the memcpy test cases.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Move the entire structure, starting from a GEP of field 0.
%struct.test01 = type { i32, i32, i32 }
define void @test01(%struct.test01* %pStructA, %struct.test01* %pStructB) !dtrans_type !2 {
  %pFieldA = getelementptr %struct.test01, %struct.test01* %pStructA, i64 0, i32 0
  %pFieldB = getelementptr %struct.test01, %struct.test01* %pStructB, i64 0, i32 0
  %pDst = bitcast i32* %pFieldA to i8*
  %pSrc = bitcast i32* %pFieldB to i8*
  call void @llvm.memmove.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 12, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Safety data: No issues found


; Move that uses a multiple of the structure size, such as for an array of structures.
%struct.test02 = type { i32, i16, i8 }
define void @test02(%struct.test02* %pStruct1, %struct.test02* %pStruct2) !dtrans_type !8 {
  %pDst = bitcast %struct.test02* %pStruct1 to i8*
  %pSrc = bitcast %struct.test02* %pStruct2 to i8*
  call void @llvm.memmove.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 80, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i8
; CHECK: Field info: Written
; CHECK: Safety data: No issues found


declare void @llvm.memmove.p0i8.p0i8.i64(i8*, i8*, i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 2, !3, !4, !4}  ; void (%struct.test01*, %struct.test01*)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{i16 0, i32 0}  ; i16
!7 = !{i8 0, i32 0}  ; i8
!8 = !{!"F", i1 false, i32 2, !3, !9, !9}  ; void (%struct.test02*, %struct.test02*)
!9 = !{!10, i32 1}  ; %struct.test02*
!10 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!11 = !{!"F", i1 false, i32 4, !3, !12, !12, !13, !14}  ; void (i8*, i8*, i64, i1)
!12 = !{i8 0, i32 1}  ; i8*
!13 = !{i64 0, i32 0}  ; i64
!14 = !{i1 0, i32 0}  ; i1
!15 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!16 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !6, !7} ; { i32, i16, i8 }
!17 = !{!"llvm.memmove.p0i8.p0i8.i64", !11}

!dtrans_types = !{!15, !16}
!dtrans_decl_types = !{!17}
