; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where an entire structure is written with a call to memcpy by
; passing a pointer to the structure to the memcpy call.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Copy the entire structure.
%struct.test01 = type { i32, i16, i8 }
define void @test01(%struct.test01* %pStruct1, %struct.test01* %pStruct2) !dtrans_type !4 {
  %pDst = bitcast %struct.test01* %pStruct1 to i8*
  %pSrc = bitcast %struct.test01* %pStruct2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 8, i1 false)
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


; Copy that uses a multiple of the structure size, such as for an array of structures.
%struct.test02 = type { i32, i16, i8 }
define void @test02(%struct.test02* %pStruct1, %struct.test02* %pStruct2) !dtrans_type !8 {
  %pDst = bitcast %struct.test02* %pStruct1 to i8*
  %pSrc = bitcast %struct.test02* %pStruct2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 80, i1 false)
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


; Copy that a multiple of the structure size, such as for an array of
; structures, where the size is not a compile time constant, but the multiplier
; is a multiple of the structure size.
%struct.test03 = type { i32, i16, i8 }
define void @test03(%struct.test03* %pStruct1, %struct.test03* %pStruct2, i32 %n) !dtrans_type !11 {
  %pDst = bitcast %struct.test03* %pStruct1 to i8*
  %pSrc = bitcast %struct.test03* %pStruct2 to i8*
  %conv = sext i32 %n to i64
  %mul = mul i64 %conv, 8
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 %mul, i1 false)
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


; Copy a structure, where the size of the copy cannot be resolved to a multiple
; of the structure size.
%struct.test04 = type { i32, i16, i8 }
define void @test04(%struct.test04* %pStruct1, %struct.test04* %pStruct2, i32 %n, i64 %sz) !dtrans_type !14 {
  %pDst = bitcast %struct.test04* %pStruct1 to i8*
  %pSrc = bitcast %struct.test04* %pStruct2 to i8*
  %conv = sext i32 %n to i64
  %mul = mul i64 %conv, %sz
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 %mul, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04
; CHECK: Safety data: Bad memfunc size{{ *$}}


; This test checks when a structure is composed of structures. Each of the
; the structure fields should be marked as written.
%struct.test05a = type { i16, i32, [2 x i32] }
%struct.test05b = type { i32, i16, i8 }
%struct.test05c = type { %struct.test05a, %struct.test05b }
define void @test05(%struct.test05c* %pStruct1, %struct.test05c* %pStruct2) !dtrans_type !21 {
  %pDst = bitcast %struct.test05c* %pStruct1 to i8*
  %pSrc = bitcast %struct.test05c* %pStruct2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 24, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05a
; CHECK: 0)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: [2 x i32]
; CHECK: Field info: Written
; CHECK: Safety data: Nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05b
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i8
; CHECK: Field info: Written
; CHECK: Safety data: Nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05c
; CHECK: Safety data: Contains nested structure{{ *$}}


; This test checks when a structure is composed of pointers to structures.
; This is a safe use, and only members of struct.test06c should be marked
; with 'Written'.
%struct.test06a = type { i16, i32, [2 x i32] }
%struct.test06b = type { i32, i16, i8 }
%struct.test06c = type { %struct.test06a*, %struct.test06b* }
define void @test06(%struct.test06c* %pStruct1, %struct.test06c* %pStruct2) !dtrans_type !28 {
  %pDst = bitcast %struct.test06c* %pStruct1 to i8*
  %pSrc = bitcast %struct.test06c* %pStruct2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 16, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test06a
; CHECK: 0)Field LLVM Type: i16
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: [2 x i32]
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: No issues found

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test06b
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
; CHECK: Name: struct.test06c
; CHECK: 0)Field LLVM Type:
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type:
; CHECK: Field info: Written
; CHECK: Safety data: No issues found


; Copy a subset of the structure using a pointer to the start of the structure.
%struct.test07 = type { i32, i32, i32, i32 }
define void @test07(%struct.test07* %pStruct1, %struct.test07* %pStruct2) !dtrans_type !31 {
  %pDst = bitcast %struct.test07* %pStruct1 to i8*
  %pSrc = bitcast %struct.test07* %pStruct2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test07
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 3)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Memfunc partial write{{ *$}}

declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i16 0, i32 0}  ; i16
!3 = !{i8 0, i32 0}  ; i8
!4 = !{!"F", i1 false, i32 2, !5, !6, !6}  ; void (%struct.test01*, %struct.test01*)
!5 = !{!"void", i32 0}  ; void
!6 = !{!7, i32 1}  ; %struct.test01*
!7 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!8 = !{!"F", i1 false, i32 2, !5, !9, !9}  ; void (%struct.test02*, %struct.test02*)
!9 = !{!10, i32 1}  ; %struct.test02*
!10 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!11 = !{!"F", i1 false, i32 3, !5, !12, !12, !1}  ; void (%struct.test03*, %struct.test03*, i32)
!12 = !{!13, i32 1}  ; %struct.test03*
!13 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!14 = !{!"F", i1 false, i32 4, !5, !15, !15, !1, !17}  ; void (%struct.test04*, %struct.test04*, i32, i64)
!15 = !{!16, i32 1}  ; %struct.test04*
!16 = !{!"R", %struct.test04 zeroinitializer, i32 0}  ; %struct.test04
!17 = !{i64 0, i32 0}  ; i64
!18 = !{!"A", i32 2, !1}  ; [2 x i32]
!19 = !{!"R", %struct.test05a zeroinitializer, i32 0}  ; %struct.test05a
!20 = !{!"R", %struct.test05b zeroinitializer, i32 0}  ; %struct.test05b
!21 = !{!"F", i1 false, i32 2, !5, !22, !22}  ; void (%struct.test05c*, %struct.test05c*)
!22 = !{!23, i32 1}  ; %struct.test05c*
!23 = !{!"R", %struct.test05c zeroinitializer, i32 0}  ; %struct.test05c
!24 = !{!25, i32 1}  ; %struct.test06a*
!25 = !{!"R", %struct.test06a zeroinitializer, i32 0}  ; %struct.test06a
!26 = !{!27, i32 1}  ; %struct.test06b*
!27 = !{!"R", %struct.test06b zeroinitializer, i32 0}  ; %struct.test06b
!28 = !{!"F", i1 false, i32 2, !5, !29, !29}  ; void (%struct.test06c*, %struct.test06c*)
!29 = !{!30, i32 1}  ; %struct.test06c*
!30 = !{!"R", %struct.test06c zeroinitializer, i32 0}  ; %struct.test06c
!31 = !{!"F", i1 false, i32 2, !5, !32, !32}  ; void (%struct.test07*, %struct.test07*)
!32 = !{!33, i32 1}  ; %struct.test07*
!33 = !{!"R", %struct.test07 zeroinitializer, i32 0}  ; %struct.test07
!34 = !{!"F", i1 false, i32 4, !5, !35, !35, !17, !36}  ; void (i8*, i8*, i64, i1)
!35 = !{i8 0, i32 1}  ; i8*
!36 = !{i1 0, i32 0}  ; i1
!37 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!38 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!39 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!40 = !{!"S", %struct.test04 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!41 = !{!"S", %struct.test05a zeroinitializer, i32 3, !2, !1, !18} ; { i16, i32, [2 x i32] }
!42 = !{!"S", %struct.test05b zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!43 = !{!"S", %struct.test05c zeroinitializer, i32 2, !19, !20} ; { %struct.test05a, %struct.test05b }
!44 = !{!"S", %struct.test06a zeroinitializer, i32 3, !2, !1, !18} ; { i16, i32, [2 x i32] }
!45 = !{!"S", %struct.test06b zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!46 = !{!"S", %struct.test06c zeroinitializer, i32 2, !24, !26} ; { %struct.test06a*, %struct.test06b* }
!47 = !{!"S", %struct.test07 zeroinitializer, i32 4, !1, !1, !1, !1} ; { i32, i32, i32, i32 }
!48 = !{!"llvm.memcpy.p0i8.p0i8.i64", !34}

!dtrans_types = !{!37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47}
!dtrans_decl_types = !{!48}
