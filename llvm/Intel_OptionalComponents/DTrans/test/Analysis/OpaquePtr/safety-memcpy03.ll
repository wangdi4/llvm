; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test the safety analysis for calls to memcpy that involve an element pointee
; for one of the pointers, when the other pointer is not an element pointee.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Test with memcpy where the source and target types match, but the source
; pointer is a field within another structure, while the destination pointer is
; not.
%struct.test01a = type { i32, i32, i32, i32, i32 }
%struct.test01b = type { i32, %struct.test01a }
define void @test01(ptr "intel_dtrans_func_index"="1" %pStructA, ptr "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !5 {
  %pField = getelementptr %struct.test01b, ptr %pStructB, i64 0, i32 1
  tail call void @llvm.memcpy.p0.p0.i64(ptr %pStructA, ptr %pField, i64 20, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01b


; Test with memcpy where the source and target types match, but the destination
; pointer is a field within another structure, while the source pointer is not.
%struct.test02a = type { i32, i32, i32, i32, i32 }
%struct.test02b = type { i32, %struct.test02a }
define void @test02(ptr "intel_dtrans_func_index"="1" %pStructA, ptr "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !9 {
  %pField = getelementptr %struct.test02b, ptr %pStructB, i64 0, i32 1
  tail call void @llvm.memcpy.p0.p0.i64(ptr %pField, ptr %pStructA, i64 20, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02a
; CHECK: Safety data: Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02b
; CHECK: Safety data: Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02b


; Test with memcpy where the source and target types do not match, when one
; pointer is an element pointee and the other is not an element pointee.
%struct.test03a = type { i32, i32, i32, i32, i32 }
%struct.test03b = type { i32, i32, i32 }
%struct.test03c = type { i32, %struct.test03b }
define void @test03(ptr "intel_dtrans_func_index"="1" %pStructA, ptr "intel_dtrans_func_index"="2" %pStructC) !intel.dtrans.func.type !13 {
  %pField = getelementptr %struct.test03c, ptr %pStructC, i64 0, i32 1
  tail call void @llvm.memcpy.p0.p0.i64(ptr %pStructA, ptr %pField, i64 12, i1 false)
  ret void
}
 ; This case is "Bad memfunc manipulation" even though one pointer is an element
 ; pointee, and the other isn't because the structure pointer types do not match.

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03a
; CHECK: Safety data: Bad memfunc manipulation{{ *$}}
; CHECK: End LLVMType: %struct.test03a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03b
; CHECK: Safety data: Bad memfunc manipulation | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test03b

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03c
; CHECK: Safety data: Bad memfunc manipulation | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test03c

declare !intel.dtrans.func.type !15 void @llvm.memcpy.p0.p0.i64(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!3 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!4 = !{%struct.test01b zeroinitializer, i32 1}  ; %struct.test01b*
!5 = distinct !{!3, !4}
!6 = !{%struct.test02a zeroinitializer, i32 0}  ; %struct.test02a
!7 = !{%struct.test02a zeroinitializer, i32 1}  ; %struct.test02a*
!8 = !{%struct.test02b zeroinitializer, i32 1}  ; %struct.test02b*
!9 = distinct !{!7, !8}
!10 = !{%struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!11 = !{%struct.test03a zeroinitializer, i32 1}  ; %struct.test03a*
!12 = !{%struct.test03c zeroinitializer, i32 1}  ; %struct.test03c*
!13 = distinct !{!11, !12}
!14 = !{i8 0, i32 1}  ; i8*
!15 = distinct !{!14, !14}
!16 = !{!"S", %struct.test01a zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!17 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !2} ; { i32, %struct.test01a }
!18 = !{!"S", %struct.test02a zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!19 = !{!"S", %struct.test02b zeroinitializer, i32 2, !1, !6} ; { i32, %struct.test02a }
!20 = !{!"S", %struct.test03a zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!21 = !{!"S", %struct.test03b zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!22 = !{!"S", %struct.test03c zeroinitializer, i32 2, !1, !10} ; { i32, %struct.test03b }

!intel.dtrans.types = !{!16, !17, !18, !19, !20, !21, !22}
