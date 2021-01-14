; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s

; Test calls to memset with a field address that is an array of structures.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This test checks using memset with a pointer to a structure that is an array
; of structure fields of a structure type. Sets all 10 elements of array of
; structure. The structure passed into the memset should be considered safe.
; Only the outer structure should be marked as a "Memfunc partial write"
%struct.test01a = type { i32, i32, i32, i32, i32 }
%struct.test01b = type { i32, [10 x %struct.test01a] }
define void @test13(%struct.test01b* %b) !dtrans_type !4 {
  %a = getelementptr inbounds %struct.test01b, %struct.test01b* %b, i64 0, i32 1
  %t0 = bitcast [10 x %struct.test01a]* %a to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %t0, i8 0, i64 200, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Memfunc partial write | Contains nested structure{{ *$}}


declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"A", i32 10, !3}  ; [10 x %struct.test01a]
!3 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!4 = !{!"F", i1 false, i32 1, !5, !6}  ; void (%struct.test01b*)
!5 = !{!"void", i32 0}  ; void
!6 = !{!7, i32 1}  ; %struct.test01b*
!7 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!8 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !2} ; { i32, [10 x %struct.test01a] }
!9 = !{!"S", %struct.test01a zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }

!dtrans_types = !{!8, !9}
