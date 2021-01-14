; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test the safety analysis for calls to memset that take the address of an
; element within a structure type.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Write the entire structure, starting from a GEP of field 0.
%struct.test01 = type { i32, i32, i32, i32, i32 }
define void @test01(%struct.test01* %pStruct) !dtrans_type !2 {
  %pField = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 0
  %pStart = bitcast i32* %pField to i8*
  call void @llvm.memset.p0i8.i64(i8* %pStart, i8 1, i64 20, i1 false)
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
; CHECK: 3)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 4)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Safety data: No issues found


; Write a subset of structure fields, starting from a GEP of field 0.
%struct.test02 = type { i32, i32, i32, i32, i32 }
define void @test02(%struct.test02* %pStruct) !dtrans_type !6 {
  %pField = getelementptr %struct.test02, %struct.test02* %pStruct, i64 0, i32 0
  %pStart = bitcast i32* %pField to i8*
  call void @llvm.memset.p0i8.i64(i8* %pStart, i8 1, i64 12, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 3)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 4)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Memfunc partial write{{ *$}}


; Write beyond the end of the structure, starting from a GEP of field 0.
%struct.test03 = type { i32, i32, i32, i32, i32 }
define void @test03(%struct.test03* %pStruct) !dtrans_type !9 {
  %pField = getelementptr %struct.test03, %struct.test03* %pStruct, i64 0, i32 0
  %pStart = bitcast i32* %pField to i8*
  call void @llvm.memset.p0i8.i64(i8* %pStart, i8 1, i64 40, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: Safety data: Bad memfunc size{{ *$}}


; Write a subset of the structure, starting and ending on a field boundary.
%struct.test04 = type { i32, i32, i32, i32, i32 }
define void @test04(%struct.test04* %pStruct) !dtrans_type !12 {
  %pField = getelementptr %struct.test04, %struct.test04* %pStruct, i64 0, i32 2
  %pStart = bitcast i32* %pField to i8*
  call void @llvm.memset.p0i8.i64(i8* %pStart, i8 1, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 3)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 4)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Memfunc partial write{{ *$}}


; Write a subset of the structure, with a size that extends beyond the structure end.
%struct.test05 = type { i32, i32, i32, i32, i32 }
define void @test05(%struct.test05* %pStruct) !dtrans_type !15 {
  %pField = getelementptr %struct.test05, %struct.test05* %pStruct, i64 0, i32 2
  %pStart = bitcast i32* %pField to i8*
  call void @llvm.memset.p0i8.i64(i8* %pStart, i8 1, i64 16, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05
; CHECK: Safety data: Bad memfunc size{{ *$}}


; Write a subset of the structure, with a size that does not end on a field boundary.
%struct.test06 = type { i32, i32, i32, i32, i32 }
define void @test06(%struct.test06* %pStruct) !dtrans_type !18 {
  %pField = getelementptr %struct.test06, %struct.test06* %pStruct, i64 0, i32 2
  %pStart = bitcast i32* %pField to i8*
  call void @llvm.memset.p0i8.i64(i8* %pStart, i8 1, i64 6, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test06
; CHECK: Safety data: Bad memfunc size{{ *$}}


declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !3, !4}  ; void (%struct.test01*)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{!"F", i1 false, i32 1, !3, !7}  ; void (%struct.test02*)
!7 = !{!8, i32 1}  ; %struct.test02*
!8 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!9 = !{!"F", i1 false, i32 1, !3, !10}  ; void (%struct.test03*)
!10 = !{!11, i32 1}  ; %struct.test03*
!11 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!12 = !{!"F", i1 false, i32 1, !3, !13}  ; void (%struct.test04*)
!13 = !{!14, i32 1}  ; %struct.test04*
!14 = !{!"R", %struct.test04 zeroinitializer, i32 0}  ; %struct.test04
!15 = !{!"F", i1 false, i32 1, !3, !16}  ; void (%struct.test05*)
!16 = !{!17, i32 1}  ; %struct.test05*
!17 = !{!"R", %struct.test05 zeroinitializer, i32 0}  ; %struct.test05
!18 = !{!"F", i1 false, i32 1, !3, !19}  ; void (%struct.test06*)
!19 = !{!20, i32 1}  ; %struct.test06*
!20 = !{!"R", %struct.test06 zeroinitializer, i32 0}  ; %struct.test06
!21 = !{!"S", %struct.test01 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!22 = !{!"S", %struct.test02 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!23 = !{!"S", %struct.test03 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!24 = !{!"S", %struct.test04 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!25 = !{!"S", %struct.test05 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!26 = !{!"S", %struct.test06 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }

!dtrans_types = !{!21, !22, !23, !24, !25, !26}
