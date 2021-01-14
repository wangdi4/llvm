; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test call to memcpy with pointer that can alias multiple types.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test01a = type { i64 }
%struct.test01b = type { i32, i32 }
define void @test01(%struct.test01b* %pStructB) !dtrans_type !3 {
  %pStructA = alloca %struct.test01a
  %pB = bitcast %struct.test01a* %pStructA to %struct.test01b*

  ; Use the pointer as %strut.test01b to infer the bitcast type.
  %field = getelementptr %struct.test01b, %struct.test01b* %pB, i64 0, i32 1
  store i32 0, i32* %field

  %pDst = bitcast %struct.test01b* %pB to i8*
  %pSrc = bitcast %struct.test01b* %pStructB to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Ambiguous GEP | Ambiguous pointer target | Local instance{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Ambiguous GEP | Ambiguous pointer target{{ *$}}


declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)


!1 = !{i64 0, i32 0}  ; i64
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!"F", i1 false, i32 1, !4, !5}  ; void (%struct.test01b*)
!4 = !{!"void", i32 0}  ; void
!5 = !{!6, i32 1}  ; %struct.test01b*
!6 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!7 = !{!"F", i1 false, i32 4, !4, !8, !8, !1, !9}  ; void (i8*, i8*, i64, i1)
!8 = !{i8 0, i32 1}  ; i8*
!9 = !{i1 0, i32 0}  ; i1
!10 = !{!"S", %struct.test01a zeroinitializer, i32 1, !1} ; { i64 }
!11 = !{!"S", %struct.test01b zeroinitializer, i32 2, !2, !2} ; { i32, i32 }
!12 = !{!"llvm.memcpy.p0i8.p0i8.i64", !7}

!dtrans_types = !{!10, !11}
!dtrans_decl_types = !{!12}
