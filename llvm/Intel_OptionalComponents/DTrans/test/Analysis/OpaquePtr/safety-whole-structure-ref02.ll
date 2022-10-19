; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test detection of 'Whole structure reference' safety condition with 'store'
; instructions.

; Test whole structure store of a member field
%struct.test01a = type { %struct.test01b }
%struct.test01b = type { i32, i32 }
define void @test01(%struct.test01a* "intel_dtrans_func_index"="1" %pStructA, %struct.test01b %structB) !intel.dtrans.func.type !4 {
  %nested = getelementptr %struct.test01a, %struct.test01a* %pStructA, i64 0, i32 0
  store %struct.test01b %structB, %struct.test01b* %nested
  ret void
}
; The "whole structure reference" should not propagate to the outer structure.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: 0)Field LLVM Type: %struct.test01b
; CHECK: Field info: Written{{ *$}}
; CHECK: Safety data: Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written{{ *$}}
; CHECK: Safety data: Whole structure reference | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01b


; Test whole structure load of a pointer that is not for a member field
%struct.test02 = type { i32, i32 }
define void @test02(%struct.test02 %struct) {
  %tmp = alloca %struct.test02
  store %struct.test02 %struct, %struct.test02* %tmp
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written{{ *$}}
; CHECK: Safety data: Whole structure reference | Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test02


!1 = !{%struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!2 = !{i32 0, i32 0}  ; i32
!3 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!4 = distinct !{!3}
!5 = !{!"S", %struct.test01a zeroinitializer, i32 1, !1} ; { %struct.test01b }
!6 = !{!"S", %struct.test01b zeroinitializer, i32 2, !2, !2} ; { i32, i32 }
!7 = !{!"S", %struct.test02 zeroinitializer, i32 2, !2, !2} ; { i32, i32 }

!intel.dtrans.types = !{!5, !6, !7}
