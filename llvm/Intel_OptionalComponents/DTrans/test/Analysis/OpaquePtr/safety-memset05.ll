; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test cases where a portion of a structure is written with a call to memset by
; passing a pointer to the structure to the memset call.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This test checks when memset only clears part of the structure, and does not
; include the padding bytes in the call to memset.
%struct.test01 = type { i32, i16, i32 }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %b) !intel.dtrans.func.type !4 {
  %p = bitcast %struct.test01* %b to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 6, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written ComplexUse
; CHECK: 1)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Memfunc partial write{{ *$}}
; CHECK: End LLVMType: %struct.test01

; This test checks when memset only writes part of the structure, and includes
; the padding bytes in the call to memset.
%struct.test02 = type { i32, i16, i32 }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %b) !intel.dtrans.func.type !6 {
  %p = bitcast %struct.test02* %b to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written ComplexUse
; CHECK: 1)Field LLVM Type: i16
; CHECK: Field info: Written ComplexUse
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Memfunc partial write{{ *$}}
; CHECK: End LLVMType: %struct.test02


declare !intel.dtrans.func.type !8 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8, i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i16 0, i32 0}  ; i16
!3 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!4 = distinct !{!3}
!5 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!6 = distinct !{!5}
!7 = !{i8 0, i32 1}  ; i8*
!8 = distinct !{!7}
!9 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, i16, i32 }
!10 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !2, !1} ; { i32, i16, i32 }

!intel.dtrans.types = !{!9, !10}
