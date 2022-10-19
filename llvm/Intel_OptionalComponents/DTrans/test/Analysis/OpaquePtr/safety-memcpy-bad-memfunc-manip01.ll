; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test calls to memcpy that trigger the "Bad memfunc manipulation" safety flag.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Cannot copy to a structure type from an arbitrary i8* pointer.
%struct.test01 = type { i32, i32, i32, i32, i32 }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStructA, i8* "intel_dtrans_func_index"="2" %pSrc) !intel.dtrans.func.type !4 {
  %pField = getelementptr %struct.test01, %struct.test01* %pStructA, i64 0, i32 0
  %pDst = bitcast i32* %pField to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 20, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Bad memfunc manipulation{{ *$}}
; CHECK: End LLVMType: %struct.test01


; Cannot copy from a structure type to a non-structure type.
%struct.test02 = type { i32, i32, i32, i32, i32 }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %pStructA, i32* "intel_dtrans_func_index"="2" %pOther) !intel.dtrans.func.type !7 {
  %pField = getelementptr %struct.test02, %struct.test02* %pStructA, i64 0, i32 0
  %pSrc = bitcast i32* %pField to i8*
  %pDst = bitcast i32* %pOther to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 20, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Bad memfunc manipulation{{ *$}}
; CHECK: End LLVMType: %struct.test02


declare !intel.dtrans.func.type !8 void @llvm.memcpy.p0i8.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2", i64, i1)


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{i8 0, i32 1}  ; i8*
!4 = distinct !{!2, !3}
!5 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!6 = !{i32 0, i32 1}  ; i32*
!7 = distinct !{!5, !6}
!8 = distinct !{!3, !3}
!9 = !{!"S", %struct.test01 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!10 = !{!"S", %struct.test02 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }

!intel.dtrans.types = !{!9, !10}
