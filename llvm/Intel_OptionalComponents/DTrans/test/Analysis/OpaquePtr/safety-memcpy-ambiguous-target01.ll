; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test calls to memcpy that have an ambiguous target. These cases should trigger
; a safety flag on the structure type.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Selecting between the address of 2 fields for the memcpy destination.
%struct.test01 = type { i32, i32, i32, i32, i32 }
define void @test01(%struct.test01* %pStructA, %struct.test01* %pStructB) !dtrans_type !2 {
  %pField0 = getelementptr %struct.test01, %struct.test01* %pStructA, i64 0, i32 0
  %pField4 = getelementptr %struct.test01, %struct.test01* %pStructA, i64 0, i32 4
  %pField = select i1 undef, i32* %pField0, i32* %pField4
  %pDst = bitcast i32* %pField to i8*

  %pSrcField0 = getelementptr %struct.test01, %struct.test01* %pStructB, i64 0, i32 0
  %pSrc = bitcast i32* %pSrcField0 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 4, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: Bad memfunc manipulation | Ambiguous pointer target{{ *$}}


; Selecting between the address of 2 fields for the memcpy source.
%struct.test02 = type { i32, i32, i32, i32, i32 }
define void @test02(%struct.test02* %pStructA, %struct.test02* %pStructB) !dtrans_type !6 {
  %pField0 = getelementptr %struct.test02, %struct.test02* %pStructA, i64 0, i32 0
  %pField4 = getelementptr %struct.test02, %struct.test02* %pStructA, i64 0, i32 4
  %pField = select i1 undef, i32* %pField0, i32* %pField4
  %pSrc = bitcast i32* %pField to i8*

  %pDstField0 = getelementptr %struct.test02, %struct.test02* %pStructB, i64 0, i32 0
  %pDst = bitcast i32* %pDstField0 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 4, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: Bad memfunc manipulation | Ambiguous pointer target{{ *$}}


declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 2, !3, !4, !4}  ; void (%struct.test01*, %struct.test01*)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{!"F", i1 false, i32 2, !3, !7, !7}  ; void (%struct.test02*, %struct.test02*)
!7 = !{!8, i32 1}  ; %struct.test02*
!8 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!9 = !{!"F", i1 false, i32 4, !3, !10, !10, !11, !12}  ; void (i8*, i8*, i64, i1)
!10 = !{i8 0, i32 1}  ; i8*
!11 = !{i64 0, i32 0}  ; i64
!12 = !{i1 0, i32 0}  ; i1
!13 = !{!"S", %struct.test01 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!14 = !{!"S", %struct.test02 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!15 = !{!"llvm.memcpy.p0i8.p0i8.i64", !9}

!dtrans_types = !{!13, !14}
!dtrans_decl_types = !{!15}
