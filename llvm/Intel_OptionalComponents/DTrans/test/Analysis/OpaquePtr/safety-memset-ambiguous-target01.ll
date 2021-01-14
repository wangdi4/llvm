; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test calls to memset that have an ambiguous target. These cases should trigger
; a safety flag on the structure type.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Selecting between the address of 2 fields.
%struct.test01 = type { i32, i32, i32, i32, i32 }
define void @test01(%struct.test01* %pStruct) !dtrans_type !2 {
  %pField0 = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 0
  %pField4 = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 4
  %pField = select i1 undef, i32* %pField0, i32* %pField4
  %pStart = bitcast i32* %pField to i8*
  call void @llvm.memset.p0i8.i64(i8* %pStart, i8 1, i64 20, i1  false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: Ambiguous pointer target{{ *$}}


; Selecting between the address of 2 fields.
%struct.test02 = type { i32, i32, i32, i32, i32 }
define void @test02(%struct.test02* %pStruct) !dtrans_type !6 {
  %pField0 = getelementptr %struct.test02, %struct.test02* %pStruct, i64 0, i32 0
  %pField4 = getelementptr %struct.test02, %struct.test02* %pStruct, i64 0, i32 4
  %pField0.p8 = bitcast i32* %pField0 to i8*
  %pField4.p8 = bitcast i32* %pField4 to i8*
  %pStart = select i1 undef, i8* %pField0.p8, i8* %pField4.p8
  call void @llvm.memset.p0i8.i64(i8* %pStart, i8 1, i64 20, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: Ambiguous pointer target{{ *$}}


declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !3, !4}  ; void (%struct.test01*)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{!"F", i1 false, i32 1, !3, !7}  ; void (%struct.test02*)
!7 = !{!8, i32 1}  ; %struct.test02*
!8 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!9 = !{!"F", i1 false, i32 4, !3, !10, !11, !12, !13}  ; void (i8*, i8, i64, i1)
!10 = !{i8 0, i32 1}  ; i8*
!11 = !{i8 0, i32 0}  ; i8
!12 = !{i64 0, i32 0}  ; i64
!13 = !{i1 0, i32 0}  ; i1
!14 = !{!"S", %struct.test01 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!15 = !{!"S", %struct.test02 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!16 = !{!"llvm.memset.p0i8.i64", !9}

!dtrans_types = !{!14, !15}
!dtrans_decl_types = !{!16}
