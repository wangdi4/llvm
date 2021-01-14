; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test calls to memcpy that trigger the "Bad memfunc manipulation" safety flag.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Cannot copy to a structure type from an arbitrary i8* pointer.
%struct.test01 = type { i32, i32, i32, i32, i32 }
define void @test01(%struct.test01* %pStructA, i8* %pSrc) !dtrans_type !2 {
  %pField = getelementptr %struct.test01, %struct.test01* %pStructA, i64 0, i32 0
  %pDst = bitcast i32* %pField to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 20, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: Bad memfunc manipulation{{ *$}}


; Cannot copy from a structure type to a non-structure type.
%struct.test02 = type { i32, i32, i32, i32, i32 }
define void @test02(%struct.test02* %pStructA, i32* %pOther) !dtrans_type !7 {
  %pField = getelementptr %struct.test02, %struct.test02* %pStructA, i64 0, i32 0
  %pSrc = bitcast i32* %pField to i8*
  %pDst = bitcast i32* %pOther to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 20, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: Bad memfunc manipulation{{ *$}}


declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 2, !3, !4, !6}  ; void (%struct.test01*, i8*)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{i8 0, i32 1}  ; i8*
!7 = !{!"F", i1 false, i32 2, !3, !8, !10}  ; void (%struct.test02*, i32*)
!8 = !{!9, i32 1}  ; %struct.test02*
!9 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!10 = !{i32 0, i32 1}  ; i32*
!11 = !{!"F", i1 false, i32 4, !3, !6, !6, !12, !13}  ; void (i8*, i8*, i64, i1)
!12 = !{i64 0, i32 0}  ; i64
!13 = !{i1 0, i32 0}  ; i1
!14 = !{!"S", %struct.test01 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!15 = !{!"S", %struct.test02 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!16 = !{!"llvm.memcpy.p0i8.p0i8.i64", !11}

!dtrans_types = !{!14, !15}
!dtrans_decl_types = !{!16}
