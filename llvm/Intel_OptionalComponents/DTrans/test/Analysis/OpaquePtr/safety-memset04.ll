; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s

; Test cases where an entire structure is written with a call to memset by
; passing a pointer to the structure to the memset call.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This test checks that all the structure fields are marked as "Written" by the
; memset call.
%struct.test01 = type { i32, i16, i8 }
define void @test01(%struct.test01* %a) !dtrans_type !4 {
  %p = bitcast %struct.test01* %a to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i8
; CHECK: Field info: Written
; CHECK: Safety data: No issues found


; This test checks when a multiple of the structure size is used, such as
; for an array of structures.
%struct.test02 = type { i32, i16, i8 }
define void @test02(%struct.test02* %a) !dtrans_type !8 {
  %p = bitcast %struct.test02* %a to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 32, i1 false)
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


; This test checks when a multiple of the structure size is used, such as for an
; array of structures, but the size is not a compile time constant.
%struct.test03 = type { i32, i16, i8 }
define void @test03(%struct.test03* %a, i32 %n) !dtrans_type !11 {
  %p = bitcast %struct.test03* %a to i8*
  %conv = sext i32 %n to i64
  %mul = mul i64 %conv, 8
  tail call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 %mul, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i8
; CHECK: Field info: Written
; CHECK: Safety data: No issues found


; This test checks when a structure is composed of structures. Each of the
; the structure fields should be marked as written.
%struct.test04a = type { i16, i32, [2 x i32] }
%struct.test04b = type { i32, i16, i8 }
%struct.test04c = type { %struct.test04a, %struct.test04b }
define void @test04(%struct.test04c* %c) !dtrans_type !17 {
  %c0 = bitcast %struct.test04c* %c to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %c0, i8 0, i64 24, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04a
; CHECK: 0)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: [2 x i32]
; CHECK: Field info: Written
; CHECK: Safety data: Nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04b
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i8
; CHECK: Field info: Written
; CHECK: Safety data: Nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04c
; CHECK: Safety data: Contains nested structure{{ *$}}


; This test checks when a structure is composed of pointers to structures.
; This is a safe use, and only members of struct.test05c should be marked
; with 'Written'.
%struct.test05a = type { i16, i32, [2 x i32] }
%struct.test05b = type { i32, i16, i8 }
%struct.test05c = type { %struct.test05a*, %struct.test05b* }
define void @test05(%struct.test05c* %c) !dtrans_type !24 {
  %c0 = bitcast %struct.test05c* %c to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %c0, i8 0, i64 16, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05a
; CHECK: 0)Field LLVM Type: i16
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: [2 x i32]
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: No issues found

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05b
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i16
; CHECK: Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: i8
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: No issues found

; Not including the actual types of the pointer fields in check lines, because
; with opaque pointers they will just be 'p0'
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05c
; CHECK: 0)Field LLVM Type:
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type:
; CHECK: Field info: Written
; CHECK: Safety data: No issues found


declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i16 0, i32 0}  ; i16
!3 = !{i8 0, i32 0}  ; i8
!4 = !{!"F", i1 false, i32 1, !5, !6}  ; void (%struct.test01*)
!5 = !{!"void", i32 0}  ; void
!6 = !{!7, i32 1}  ; %struct.test01*
!7 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!8 = !{!"F", i1 false, i32 1, !5, !9}  ; void (%struct.test02*)
!9 = !{!10, i32 1}  ; %struct.test02*
!10 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!11 = !{!"F", i1 false, i32 2, !5, !12, !1}  ; void (%struct.test03*, i32)
!12 = !{!13, i32 1}  ; %struct.test03*
!13 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!14 = !{!"A", i32 2, !1}  ; [2 x i32]
!15 = !{!"R", %struct.test04a zeroinitializer, i32 0}  ; %struct.test04a
!16 = !{!"R", %struct.test04b zeroinitializer, i32 0}  ; %struct.test04b
!17 = !{!"F", i1 false, i32 1, !5, !18}  ; void (%struct.test04c*)
!18 = !{!19, i32 1}  ; %struct.test04c*
!19 = !{!"R", %struct.test04c zeroinitializer, i32 0}  ; %struct.test04c
!20 = !{!21, i32 1}  ; %struct.test05a*
!21 = !{!"R", %struct.test05a zeroinitializer, i32 0}  ; %struct.test05a
!22 = !{!23, i32 1}  ; %struct.test05b*
!23 = !{!"R", %struct.test05b zeroinitializer, i32 0}  ; %struct.test05b
!24 = !{!"F", i1 false, i32 1, !5, !25}  ; void (%struct.test05c*)
!25 = !{!26, i32 1}  ; %struct.test05c*
!26 = !{!"R", %struct.test05c zeroinitializer, i32 0}  ; %struct.test05c
!27 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!28 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!29 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!30 = !{!"S", %struct.test04a zeroinitializer, i32 3, !2, !1, !14} ; { i16, i32, [2 x i32] }
!31 = !{!"S", %struct.test04b zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!32 = !{!"S", %struct.test04c zeroinitializer, i32 2, !15, !16} ; { %struct.test04a, %struct.test04b }
!33 = !{!"S", %struct.test05a zeroinitializer, i32 3, !2, !1, !14} ; { i16, i32, [2 x i32] }
!34 = !{!"S", %struct.test05b zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!35 = !{!"S", %struct.test05c zeroinitializer, i32 2, !20, !22} ; { %struct.test05a*, %struct.test05b* }

!dtrans_types = !{!27, !28, !29, !30, !31, !32, !33, !34, !35}
