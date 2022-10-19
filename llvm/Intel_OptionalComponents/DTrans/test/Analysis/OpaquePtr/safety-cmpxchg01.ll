; REQUIRES: asserts
; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Check that a structure type that is used, or has the address of a field
; member used  as the pointer operand of a 'cmpxchg' instruction gets
; marked as 'Unhandled use' because the safety analyzer does not support
; tracking the values used for field values in these instructions, or checking
; whether types matches the expected types.

; Check that a pointer to a field member used in a 'cmpxchg' instruction causes
; the structure that contains it to be marked as 'Unhandled use'
%struct.test1 = type { i64, i64 }
define void @test1() {
  %var1 = alloca %struct.test1
  %f0 = getelementptr %struct.test1, ptr %var1, i64 0, i32 0
  %r0 = cmpxchg volatile ptr %f0, i64 0, i64 1 seq_cst seq_cst
  ret void
}

; Test with GEP-less variation for a structure field. This should
; cause the structure to be marked as 'Unhandled use'
%struct.test2 = type { i64, i64 }
define void @test2() {
  %var2 = alloca %struct.test2
  %r0 = cmpxchg volatile ptr %var2, i64 0, i64 1 seq_cst seq_cst
  ret void
}

; Test with a pointer to a structure.  This should cause the
; structure to be marked as 'Unhandled use'
%struct.test3 = type { i64, i64 }
define void @test3() {
  %var3 = alloca ptr, !intel_dtrans_type !2
  %l = alloca ptr, !intel_dtrans_type !2
  %r1 = cmpxchg volatile ptr %var3, ptr null, ptr %l seq_cst seq_cst
  ret void
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test1
; CHECK: Safety data: Local instance | Unhandled use
; CHECK: End LLVMType: %struct.test1

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test2
; CHECK: Safety data: Local instance | Unhandled use
; CHECK: End LLVMType: %struct.test2

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test3
; CHECK: Safety data: Local pointer | Unhandled use
; CHECK: End LLVMType: %struct.test3

!intel.dtrans.types = !{!3, !4, !5}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test3 zeroinitializer, i32 1}  ; %struct.test3*
!3 = !{!"S", %struct.test1 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!4 = !{!"S", %struct.test2 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!5 = !{!"S", %struct.test3 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }

