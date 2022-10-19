; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test setting of "Global pointer" safety bit.

%struct.test01 = type { i32, i32 }
@global_ptr_to_struct = internal global %struct.test01* zeroinitializer, !intel_dtrans_type !2
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Global pointer
; CHECK: End LLVMType: %struct.test01

%struct.test02 = type { i32, i32 }
@global_ptr_to_ptr_to_struct = internal global %struct.test02** zeroinitializer, !intel_dtrans_type !3

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Global pointer
; CHECK: End LLVMType: %struct.test02

%struct.test03a = type { %struct.test03b, %struct.test03c* }
%struct.test03b = type { i32, i32 }
%struct.test03c = type { i32, i32 }
@global_ptr_struct3 =  internal global %struct.test03a* zeroinitializer, !intel_dtrans_type !6
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03a
; CHECK: Safety data: Global pointer
; CHECK: End LLVMType: %struct.test03a

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03b
; CHECK: Safety data: Global pointer
; CHECK: End LLVMType: %struct.test03b

; "Global pointer" is not pointer-carried to referenced types.
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03c
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test03c

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{%struct.test02 zeroinitializer, i32 2}  ; %struct.test02**
!4 = !{%struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!5 = !{%struct.test03c zeroinitializer, i32 1}  ; %struct.test03c*
!6 = !{%struct.test03a zeroinitializer, i32 1}  ; %struct.test03a*
!7 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!8 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!9 = !{!"S", %struct.test03a zeroinitializer, i32 2, !4, !5} ; { %struct.test03b, %struct.test03c* }
!10 = !{!"S", %struct.test03b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!11 = !{!"S", %struct.test03c zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!7, !8, !9, !10, !11}
