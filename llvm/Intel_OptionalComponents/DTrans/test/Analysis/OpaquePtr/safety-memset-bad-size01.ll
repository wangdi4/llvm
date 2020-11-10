; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s

; Test that calls to memset with a size parameter that is not supported by
; DTrans are marked with "Bad memfunc size"

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Test with element pointee when calling memset using a runtime dependent size.
%struct.test01 = type { i32, i32, i32, i32 }
define void @test01(%struct.test01* %pStruct, i64 %size) !dtrans_type !2 {
  %pField = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 0
  %pStart = bitcast i32* %pField to i8*
  call void @llvm.memset.p0i8.i64(i8* %pStart, i8 1, i64 %size, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: Bad memfunc size{{ *$}}


; Test with pointer to structure type when calling memset using a runtime
; dependent size.
%struct.test02 = type { i32, i32, i32, i32 }
define void @test02(%struct.test02* %a, i64 %size) !dtrans_type !7 {
  %p = bitcast %struct.test02* %a to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 %size, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: Bad memfunc size{{ *$}}


declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 2, !3, !4, !6}  ; void (%struct.test01*, i64)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{i64 0, i32 0}  ; i64
!7 = !{!"F", i1 false, i32 2, !3, !8, !6}  ; void (%struct.test02*, i64)
!8 = !{!9, i32 1}  ; %struct.test02*
!9 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!10 = !{!"F", i1 false, i32 4, !3, !11, !12, !6, !13}  ; void (i8*, i8, i64, i1)
!11 = !{i8 0, i32 1}  ; i8*
!12 = !{i8 0, i32 0}  ; i8
!13 = !{i1 0, i32 0}  ; i1
!14 = !{!"S", %struct.test01 zeroinitializer, i32 4, !1, !1, !1, !1} ; { i32, i32, i32, i32 }
!15 = !{!"S", %struct.test02 zeroinitializer, i32 4, !1, !1, !1, !1} ; { i32, i32, i32, i32 }
!16 = !{!"llvm.memset.p0i8.i64", !10}

!dtrans_types = !{!14, !15}
!dtrans_decl_types = !{!16}
