; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test calls to memcpy that trigger the "Bad memfunc manipulation" safety flag.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Cannot copy to a structure type from an arbitrary i8* pointer.
%struct.test01 = type { i32, i32, i32, i32, i32 }
define void @test01(ptr "intel_dtrans_func_index"="1" %pStructA, ptr "intel_dtrans_func_index"="2" %pSrc) !intel.dtrans.func.type !4 {
  %pField = getelementptr %struct.test01, ptr %pStructA, i64 0, i32 0
  call void @llvm.memcpy.p0.p0.i64(ptr %pField, ptr %pSrc, i64 20, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Bad memfunc manipulation{{ *$}}
; CHECK: End LLVMType: %struct.test01


; Cannot copy from a structure type to a non-structure type.
%struct.test02 = type { i32, i32, i32, i32, i32 }
define void @test02(ptr "intel_dtrans_func_index"="1" %pStructA, ptr "intel_dtrans_func_index"="2" %pOther) !intel.dtrans.func.type !7 {
  %pField = getelementptr %struct.test02, ptr %pStructA, i64 0, i32 0
  call void @llvm.memcpy.p0.p0.i64(ptr %pOther, ptr %pField, i64 20, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Bad memfunc manipulation{{ *$}}
; CHECK: End LLVMType: %struct.test02


declare !intel.dtrans.func.type !8 void @llvm.memcpy.p0.p0.i64(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", i64, i1)


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
