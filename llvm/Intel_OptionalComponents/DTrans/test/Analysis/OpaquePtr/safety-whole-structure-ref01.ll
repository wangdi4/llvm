; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test detection of 'Whole structure reference' safety condition on 'load'
; instructions.

; Test whole structure load of a member field
%struct.test01a = type { %struct.test01b }
%struct.test01b = type { i32, i32 }
define void @test01(%struct.test01a* "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !4 {
  %nested = getelementptr %struct.test01a, %struct.test01a* %p, i64 0, i32 0
  %t = load %struct.test01b, %struct.test01b* %nested
  ret void
}
; The "whole structure reference" should not propagate to the outer structure.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: 0)Field LLVM Type: %struct.test01b
; CHECK: Field info: Read{{ *$}}
; CHECK: Safety data: Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Read{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Read{{ *$}}
; CHECK: Safety data: Whole structure reference | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01b


; Test whole structure load of a pointer that is not for a member field
%struct.test02 = type { i32, i32 }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !6 {
  %t = load %struct.test02, %struct.test02* %p
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Read{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Read{{ *$}}
; CHECK: Safety data: Whole structure reference{{ *$}}
; CHECK: End LLVMType: %struct.test02


!1 = !{%struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!2 = !{i32 0, i32 0}  ; i32
!3 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!4 = distinct !{!3}
!5 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!6 = distinct !{!5}
!7 = !{!"S", %struct.test01a zeroinitializer, i32 1, !1} ; { %struct.test01b }
!8 = !{!"S", %struct.test01b zeroinitializer, i32 2, !2, !2} ; { i32, i32 }
!9 = !{!"S", %struct.test02 zeroinitializer, i32 2, !2, !2} ; { i32, i32 }

!intel.dtrans.types = !{!7, !8, !9}
