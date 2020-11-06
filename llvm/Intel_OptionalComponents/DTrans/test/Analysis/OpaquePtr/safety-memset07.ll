; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s

; Test calls to memset using byte-flattened GEP addressing.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Call memset using a byte address that is field start.
%struct.test01 = type { i32, i64, i32, i16, i16 } ; Offsets: 0, 8, 16, 20, 22
define %struct.test01* @test01() !dtrans_type !4 {
  %mem = call i8* @malloc(i64 24)
  %addr0 = getelementptr i8, i8* %mem, i32 8
  call void @llvm.memset.p0i8.i64(i8* %addr0, i8 1, i64 16, i1 false)
  %newStruct = bitcast i8* %mem to %struct.test01*
  ret %struct.test01* %newStruct
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i64
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 3)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: 4)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: Safety data: Memfunc partial write{{ *$}}


; Call memset starting with the address of a padding byte between fields.
%struct.test02 = type { i32, i64, i32, i16, i16 } ; Offsets: 0, 8, 16, 20, 22
define %struct.test02* @test02() !dtrans_type !7 {
  %mem = call i8* @malloc(i64 24)
  %addr0 = getelementptr i8, i8* %mem, i32 4
  call void @llvm.memset.p0i8.i64(i8* %addr0, i8 1, i64 20, i1 false)
  %newStruct = bitcast i8* %mem to %struct.test02*
  ret %struct.test02* %newStruct
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i64
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 3)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: 4)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: Safety data: Memfunc partial write{{ *$}}


; Call memset starting with a byte that is not padding or a field boundary.
%struct.test03 = type { i32, i64, i32, i16, i16 } ; Offsets: 0, 8, 16, 20, 22
define %struct.test03* @test03() !dtrans_type !10 {
  %mem = call i8* @malloc(i64 24)
  %addr0 = getelementptr i8, i8* %mem, i32 12
  call void @llvm.memset.p0i8.i64(i8* %addr0, i8 1, i64 12, i1 false)
  %newStruct = bitcast i8* %mem to %struct.test03*
  ret %struct.test03* %newStruct
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: Safety data: Bad memfunc size{{ *$}}


declare i8* @malloc(i64)
declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{!"F", i1 false, i32 0, !5}  ; %struct.test01* ()
!5 = !{!6, i32 1}  ; %struct.test01*
!6 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!7 = !{!"F", i1 false, i32 0, !8}  ; %struct.test02* ()
!8 = !{!9, i32 1}  ; %struct.test02*
!9 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!10 = !{!"F", i1 false, i32 0, !11}  ; %struct.test03* ()
!11 = !{!12, i32 1}  ; %struct.test03*
!12 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!13 = !{!"S", %struct.test01 zeroinitializer, i32 5, !1, !2, !1, !3, !3} ; { i32, i64, i32, i16, i16 }
!14 = !{!"S", %struct.test02 zeroinitializer, i32 5, !1, !2, !1, !3, !3} ; { i32, i64, i32, i16, i16 }
!15 = !{!"S", %struct.test03 zeroinitializer, i32 5, !1, !2, !1, !3, !3} ; { i32, i64, i32, i16, i16 }

!dtrans_types = !{!13, !14, !15}
