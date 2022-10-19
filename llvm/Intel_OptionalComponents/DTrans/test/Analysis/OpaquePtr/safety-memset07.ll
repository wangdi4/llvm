; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test calls to memset using byte-flattened GEP addressing.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Call memset using a byte address that is field start.
%struct.test01 = type { i32, i64, i32, i16, i16 } ; Offsets: 0, 8, 16, 20, 22
define "intel_dtrans_func_index"="1" %struct.test01* @test01() !intel.dtrans.func.type !5 {
  %mem = call i8* @malloc(i64 24)
  %addr0 = getelementptr i8, i8* %mem, i32 8
  call void @llvm.memset.p0i8.i64(i8* %addr0, i8 1, i64 16, i1 false)
  %newStruct = bitcast i8* %mem to %struct.test01*
  ret %struct.test01* %newStruct
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i64
; CHECK: Field info: Written ComplexUse
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Written ComplexUse
; CHECK: 3)Field LLVM Type: i16
; CHECK: Field info: Written ComplexUse
; CHECK: 4)Field LLVM Type: i16
; CHECK: Field info: Written ComplexUse
; CHECK: Safety data: Memfunc partial write{{ *$}}
; CHECK: End LLVMType: %struct.test01


; Call memset starting with the address of a padding byte between fields.
%struct.test02 = type { i32, i64, i32, i16, i16 } ; Offsets: 0, 8, 16, 20, 22
define "intel_dtrans_func_index"="1" %struct.test02* @test02() !intel.dtrans.func.type !7 {
  %mem = call i8* @malloc(i64 24)
  %addr0 = getelementptr i8, i8* %mem, i32 4
  call void @llvm.memset.p0i8.i64(i8* %addr0, i8 1, i64 20, i1 false)
  %newStruct = bitcast i8* %mem to %struct.test02*
  ret %struct.test02* %newStruct
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i64
; CHECK: Field info: Written ComplexUse
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Written ComplexUse
; CHECK: 3)Field LLVM Type: i16
; CHECK: Field info: Written ComplexUse
; CHECK: 4)Field LLVM Type: i16
; CHECK: Field info: Written ComplexUse
; CHECK: Safety data: Memfunc partial write{{ *$}}
; CHECK: End LLVMType: %struct.test02


; Call memset starting with a byte that is not padding or a field boundary.
%struct.test03 = type { i32, i64, i32, i16, i16 } ; Offsets: 0, 8, 16, 20, 22
define "intel_dtrans_func_index"="1" %struct.test03* @test03() !intel.dtrans.func.type !9 {
  %mem = call i8* @malloc(i64 24)
  %addr0 = getelementptr i8, i8* %mem, i32 12
  call void @llvm.memset.p0i8.i64(i8* %addr0, i8 1, i64 12, i1 false)
  %newStruct = bitcast i8* %mem to %struct.test03*
  ret %struct.test03* %newStruct
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: Bad memfunc size{{ *$}}
; CHECK: End LLVMType: %struct.test03


declare !intel.dtrans.func.type !11 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0
declare !intel.dtrans.func.type !12 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8, i64, i1)

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!5 = distinct !{!4}
!6 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!7 = distinct !{!6}
!8 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!9 = distinct !{!8}
!10 = !{i8 0, i32 1}  ; i8*
!11 = distinct !{!10}
!12 = distinct !{!10}
!13 = !{!"S", %struct.test01 zeroinitializer, i32 5, !1, !2, !1, !3, !3} ; { i32, i64, i32, i16, i16 }
!14 = !{!"S", %struct.test02 zeroinitializer, i32 5, !1, !2, !1, !3, !3} ; { i32, i64, i32, i16, i16 }
!15 = !{!"S", %struct.test03 zeroinitializer, i32 5, !1, !2, !1, !3, !3} ; { i32, i64, i32, i16, i16 }

!intel.dtrans.types = !{!13, !14, !15}
