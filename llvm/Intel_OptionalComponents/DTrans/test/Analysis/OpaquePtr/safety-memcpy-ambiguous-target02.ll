; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test call to memcpy with pointer that can alias multiple types.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test01a = type { i64 }
%struct.test01b = type { i32, i32 }
define void @test01(%struct.test01b* "intel_dtrans_func_index"="1" %pStructB) !intel.dtrans.func.type !4 {
  %pStructA = alloca %struct.test01a
  %pB = bitcast %struct.test01a* %pStructA to %struct.test01b*

  ; Use the pointer as %strut.test01b to infer the bitcast type.
  %field = getelementptr %struct.test01b, %struct.test01b* %pB, i64 0, i32 1
  store i32 0, i32* %field

  %pDst = bitcast %struct.test01b* %pB to i8*
  %pSrc = bitcast %struct.test01b* %pStructB to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Ambiguous GEP | Ambiguous pointer target | Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Ambiguous GEP | Ambiguous pointer target{{ *$}}
; CHECK: End LLVMType: %struct.test01b

declare !intel.dtrans.func.type !6 void @llvm.memcpy.p0i8.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2", i64, i1)


!1 = !{i64 0, i32 0}  ; i64
!2 = !{i32 0, i32 0}  ; i32
!3 = !{%struct.test01b zeroinitializer, i32 1}  ; %struct.test01b*
!4 = distinct !{!3}
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5, !5}
!7 = !{!"S", %struct.test01a zeroinitializer, i32 1, !1} ; { i64 }
!8 = !{!"S", %struct.test01b zeroinitializer, i32 2, !2, !2} ; { i32, i32 }

!intel.dtrans.types = !{!7, !8}
