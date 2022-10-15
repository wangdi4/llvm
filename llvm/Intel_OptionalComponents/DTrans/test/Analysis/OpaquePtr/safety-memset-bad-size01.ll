; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s

; Test that calls to memset with a size parameter that is not supported by
; DTrans are marked with "Bad memfunc size"

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Test with element pointee when calling memset using a runtime dependent size.
%struct.test01 = type { i32, i32, i32, i32 }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStruct, i64 %size) !intel.dtrans.func.type !3 {
  %pField = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 0
  %pStart = bitcast i32* %pField to i8*
  call void @llvm.memset.p0i8.i64(i8* %pStart, i8 1, i64 %size, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Bad memfunc size{{ *$}}
; CHECK: End LLVMType: %struct.test01


; Test with pointer to structure type when calling memset using a runtime
; dependent size.
%struct.test02 = type { i32, i32, i32, i32 }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %a, i64 %size) !intel.dtrans.func.type !5 {
  %p = bitcast %struct.test02* %a to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 %size, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Bad memfunc size{{ *$}}
; CHECK: End LLVMType: %struct.test02


declare !intel.dtrans.func.type !7 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8, i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4}
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 4, !1, !1, !1, !1} ; { i32, i32, i32, i32 }
!9 = !{!"S", %struct.test02 zeroinitializer, i32 4, !1, !1, !1, !1} ; { i32, i32, i32, i32 }

!intel.dtrans.types = !{!8, !9}
