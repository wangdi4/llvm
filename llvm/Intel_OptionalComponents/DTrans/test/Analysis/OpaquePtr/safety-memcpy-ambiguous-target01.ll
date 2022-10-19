; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test calls to memcpy that have an ambiguous target. These cases should trigger
; a safety flag on the structure type.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Selecting between the address of 2 fields for the memcpy destination.
%struct.test01 = type { i32, i32, i32, i32, i32 }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStructA, %struct.test01* "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !3 {
  %pField0 = getelementptr %struct.test01, %struct.test01* %pStructA, i64 0, i32 0
  %pField4 = getelementptr %struct.test01, %struct.test01* %pStructA, i64 0, i32 4
  %pField = select i1 undef, i32* %pField0, i32* %pField4
  %pDst = bitcast i32* %pField to i8*

  %pSrcField0 = getelementptr %struct.test01, %struct.test01* %pStructB, i64 0, i32 0
  %pSrc = bitcast i32* %pSrcField0 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 4, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Bad memfunc manipulation | Ambiguous pointer target{{ *$}}
; CHECK: End LLVMType: %struct.test01


; Selecting between the address of 2 fields for the memcpy source.
%struct.test02 = type { i32, i32, i32, i32, i32 }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %pStructA, %struct.test02* "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !5 {
  %pField0 = getelementptr %struct.test02, %struct.test02* %pStructA, i64 0, i32 0
  %pField4 = getelementptr %struct.test02, %struct.test02* %pStructA, i64 0, i32 4
  %pField = select i1 undef, i32* %pField0, i32* %pField4
  %pSrc = bitcast i32* %pField to i8*

  %pDstField0 = getelementptr %struct.test02, %struct.test02* %pStructB, i64 0, i32 0
  %pDst = bitcast i32* %pDstField0 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 4, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Bad memfunc manipulation | Ambiguous pointer target{{ *$}}
; CHECK: End LLVMType: %struct.test02

declare !intel.dtrans.func.type !7 void @llvm.memcpy.p0i8.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2", i64, i1)


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
