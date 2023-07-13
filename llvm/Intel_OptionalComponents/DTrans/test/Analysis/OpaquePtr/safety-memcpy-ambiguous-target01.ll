; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test calls to memcpy that have an ambiguous target. These cases should trigger
; a safety flag on the structure type.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Selecting between the address of 2 fields for the memcpy destination.
%struct.test01 = type { i32, i32, i32, i32, i32 }
define void @test01(ptr "intel_dtrans_func_index"="1" %pStructA, ptr "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !3 {
  %pField0 = getelementptr %struct.test01, ptr %pStructA, i64 0, i32 0
  %pField4 = getelementptr %struct.test01, ptr %pStructA, i64 0, i32 4
  %pField = select i1 undef, ptr %pField0, ptr %pField4

  %pSrcField0 = getelementptr %struct.test01, ptr %pStructB, i64 0, i32 0
  call void @llvm.memcpy.p0.p0.i64(ptr %pField, ptr %pSrcField0, i64 4, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Bad memfunc manipulation | Ambiguous pointer target{{ *$}}
; CHECK: End LLVMType: %struct.test01


; Selecting between the address of 2 fields for the memcpy source.
%struct.test02 = type { i32, i32, i32, i32, i32 }
define void @test02(ptr "intel_dtrans_func_index"="1" %pStructA, ptr "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !5 {
  %pField0 = getelementptr %struct.test02, ptr %pStructA, i64 0, i32 0
  %pField4 = getelementptr %struct.test02, ptr %pStructA, i64 0, i32 4
  %pField = select i1 undef, ptr %pField0, ptr %pField4

  %pDstField0 = getelementptr %struct.test02, ptr %pStructB, i64 0, i32 0
  call void @llvm.memcpy.p0.p0.i64(ptr %pDstField0, ptr %pField, i64 4, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Bad memfunc manipulation | Ambiguous pointer target{{ *$}}
; CHECK: End LLVMType: %struct.test02

declare !intel.dtrans.func.type !7 void @llvm.memcpy.p0.p0.i64(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", i64, i1)


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2, !2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4, !4}
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6, !6}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!9 = !{!"S", %struct.test02 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }

!intel.dtrans.types = !{!8, !9}
