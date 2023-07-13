; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases for safety analysis of memmove calls. The logic for calls to
; memmove is the same as memcpy, so this test just does sanity checking that
; memmove is recognized. For more complete testing of safety conditions that may
; impact memmove calls, refer to the memcpy test cases.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Move the entire structure, starting from a GEP of field 0.
%struct.test01 = type { i32, i32, i32 }
define void @test01(ptr "intel_dtrans_func_index"="1" %pStructA, ptr "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !3 {
  %pFieldA = getelementptr %struct.test01, ptr %pStructA, i64 0, i32 0
  %pFieldB = getelementptr %struct.test01, ptr %pStructB, i64 0, i32 0
  call void @llvm.memmove.p0.p0.i64(ptr %pFieldA, ptr %pFieldB, i64 12, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01


; Move that uses a multiple of the structure size, such as for an array of structures.
%struct.test02 = type { i32, i16, i8 }
define void @test02(ptr "intel_dtrans_func_index"="1" %pStruct1, ptr "intel_dtrans_func_index"="2" %pStruct2) !intel.dtrans.func.type !7 {
  call void @llvm.memmove.p0.p0.i64(ptr %pStruct1, ptr %pStruct2, i64 80, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i8
; CHECK: Field info: Written
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test02


declare !intel.dtrans.func.type !9 void @llvm.memmove.p0.p0.i64(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2, !2}
!4 = !{i16 0, i32 0}  ; i16
!5 = !{i8 0, i32 0}  ; i8
!6 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!7 = distinct !{!6, !6}
!8 = !{i8 0, i32 1}  ; i8*
!9 = distinct !{!8, !8}
!10 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!11 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !4, !5} ; { i32, i16, i8 }

!intel.dtrans.types = !{!10, !11}
