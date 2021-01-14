; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s

; Test cases where a portion of a structure is written with a call to memset by
; passing a pointer to the structure to the memset call.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This test checks when memset only clears part of the structure, and does not
; include the padding bytes in the call to memset.
%struct.test01 = type { i32, i16, i32 }
define void @test01(%struct.test01* %b) !dtrans_type !3 {
  %p = bitcast %struct.test01* %b to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 6, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Memfunc partial write{{ *$}}

; This test checks when memset only writes part of the structure, and includes
; the padding bytes in the call to memset.
%struct.test02 = type { i32, i16, i32 }
define void @test02(%struct.test02* %b) !dtrans_type !7 {
  %p = bitcast %struct.test02* %b to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Memfunc partial write{{ *$}}


declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i16 0, i32 0}  ; i16
!3 = !{!"F", i1 false, i32 1, !4, !5}  ; void (%struct.test01*)
!4 = !{!"void", i32 0}  ; void
!5 = !{!6, i32 1}  ; %struct.test01*
!6 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!7 = !{!"F", i1 false, i32 1, !4, !8}  ; void (%struct.test02*)
!8 = !{!9, i32 1}  ; %struct.test02*
!9 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!10 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, i16, i32 }
!11 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !2, !1} ; { i32, i16, i32 }

!dtrans_types = !{!10, !11}
