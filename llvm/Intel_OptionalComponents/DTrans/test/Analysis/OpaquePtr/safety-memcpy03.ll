; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test the safety analysis for calls to memcpy that involve an element pointee
; for one of the pointers, when the other pointer is not an element pointee.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Test with memcpy where the source and target types match, but the source
; pointer is a field within another structure, while the destination pointer is
; not.
%struct.test01a = type { i32, i32, i32, i32, i32 }
%struct.test01b = type { i32, %struct.test01a }
define void @test01(%struct.test01a* %pStructA, %struct.test01b* %pStructB) !dtrans_type !3 {
  %pDst = bitcast %struct.test01a* %pStructA to i8*
  %pField = getelementptr %struct.test01b, %struct.test01b* %pStructB, i64 0, i32 1
  %pSrc = bitcast %struct.test01a* %pField to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 20, i1 false)
  ret void
}
; TODO: Copying to/from a nested structure element to a pointer of the same type
; is not supported yet. It may be needed for deepsjeng, in the future, at which
; point this should no longer get marked as "Bad memfunc manipulation".

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Bad memfunc manipulation | Nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Bad memfunc manipulation | Contains nested structure{{ *$}}


; Test with memcpy where the source and target types match, but the destination
; pointer is a field within another structure, while the source pointer is not.
%struct.test02a = type { i32, i32, i32, i32, i32 }
%struct.test02b = type { i32, %struct.test02a }
define void @test02(%struct.test02a* %pStructA, %struct.test02b* %pStructB) !dtrans_type !9 {
  %pSrc = bitcast %struct.test02a* %pStructA to i8*
  %pField = getelementptr %struct.test02b, %struct.test02b* %pStructB, i64 0, i32 1
  %pDst = bitcast %struct.test02a* %pField to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 20, i1 false)
  ret void
}
; TODO: Copying to/from a nested structure element to a pointer of the same type
; is not supported yet. It may be needed for deepsjeng, in the future, at which
; point this should no longer get marked as "Bad memfunc manipulation".

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02a
; CHECK: Safety data: Bad memfunc manipulation | Nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02b
; CHECK: Safety data: Bad memfunc manipulation | Contains nested structure{{ *$}}


; Test with memcpy where the source and target types do not match, when one
; pointer is an element pointee and the other is not an element pointee.
%struct.test03a = type { i32, i32, i32, i32, i32 }
%struct.test03b = type { i32, i32, i32 }
%struct.test03c = type { i32, %struct.test03b }
define void @test03(%struct.test03a* %pStructA, %struct.test03c* %pStructC) !dtrans_type !14 {
  %pDst = bitcast %struct.test03a* %pStructA to i8*
  %pField = getelementptr %struct.test03c, %struct.test03c* %pStructC, i64 0, i32 1
  %pSrc = bitcast %struct.test03b* %pField to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 12, i1 false)
  ret void
}
 ; This case will still be "Bad memfunc manipulation" after support of element
 ; pointee and non-element pointees is implemented.

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03a
; CHECK: Safety data: Bad memfunc manipulation{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03b
; CHECK: Safety data: Bad memfunc manipulation | Nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03c
; CHECK: Safety data: Bad memfunc manipulation | Contains nested structure{{ *$}}

declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!3 = !{!"F", i1 false, i32 2, !4, !5, !6}  ; void (%struct.test01a*, %struct.test01b*)
!4 = !{!"void", i32 0}  ; void
!5 = !{!2, i32 1}  ; %struct.test01a*
!6 = !{!7, i32 1}  ; %struct.test01b*
!7 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!8 = !{!"R", %struct.test02a zeroinitializer, i32 0}  ; %struct.test02a
!9 = !{!"F", i1 false, i32 2, !4, !10, !11}  ; void (%struct.test02a*, %struct.test02b*)
!10 = !{!8, i32 1}  ; %struct.test02a*
!11 = !{!12, i32 1}  ; %struct.test02b*
!12 = !{!"R", %struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!13 = !{!"R", %struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!14 = !{!"F", i1 false, i32 2, !4, !15, !17}  ; void (%struct.test03a*, %struct.test03c*)
!15 = !{!16, i32 1}  ; %struct.test03a*
!16 = !{!"R", %struct.test03a zeroinitializer, i32 0}  ; %struct.test03a
!17 = !{!18, i32 1}  ; %struct.test03c*
!18 = !{!"R", %struct.test03c zeroinitializer, i32 0}  ; %struct.test03c
!19 = !{!"F", i1 false, i32 4, !4, !20, !20, !21, !22}  ; void (i8*, i8*, i64, i1)
!20 = !{i8 0, i32 1}  ; i8*
!21 = !{i64 0, i32 0}  ; i64
!22 = !{i1 0, i32 0}  ; i1
!23 = !{!"S", %struct.test01a zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!24 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !2} ; { i32, %struct.test01a }
!25 = !{!"S", %struct.test02a zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!26 = !{!"S", %struct.test02b zeroinitializer, i32 2, !1, !8} ; { i32, %struct.test02a }
!27 = !{!"S", %struct.test03a zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!28 = !{!"S", %struct.test03b zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!29 = !{!"S", %struct.test03c zeroinitializer, i32 2, !1, !13} ; { i32, %struct.test03b }
!30 = !{!"llvm.memcpy.p0i8.p0i8.i64", !19}

!dtrans_types = !{!23, !24, !25, !26, !27, !28, !29}
!dtrans_decl_types = !{!30}
