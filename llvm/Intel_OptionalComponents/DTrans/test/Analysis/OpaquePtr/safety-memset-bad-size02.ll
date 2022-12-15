; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s

; Test that calls to memset with a size parameter that is not supported by
; DTrans are marked with "Bad memfunc size"

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This test checks using memset with a pointer to a structure that is an array
; of structures field of a structure. However, this version only sets 5
; elements of array of structure, resulting on the "Bed memfunc size" being set
; on the structure containing the field. This will also cascade to the nested
; structure.
%struct.test01a = type { i32, i32, i32, i32, i32 }
%struct.test01b = type { i32, [10 x %struct.test01a] }
define void @test14(ptr "intel_dtrans_func_index"="1" %b) !intel.dtrans.func.type !5 {
  %a = getelementptr inbounds %struct.test01b, ptr %b, i64 0, i32 1
  %t0 = bitcast ptr %a to ptr
  tail call void @llvm.memset.p0.i64(ptr %t0, i8 0, i64 100, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Bad memfunc size | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Bad memfunc size | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01b


declare !intel.dtrans.func.type !7 void @llvm.memset.p0.i64(ptr "intel_dtrans_func_index"="1", i8, i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"A", i32 10, !3}  ; [10 x %struct.test01a]
!3 = !{%struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!4 = !{%struct.test01b zeroinitializer, i32 1}  ; %struct.test01b*
!5 = distinct !{!4}
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test01a zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!9 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !2} ; { i32, [10 x %struct.test01a] }

!intel.dtrans.types = !{!8, !9}
