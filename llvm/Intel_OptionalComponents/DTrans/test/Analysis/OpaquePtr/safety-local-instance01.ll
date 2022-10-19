; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test setting of "Local instance" safety bit


; This case should set "Local instance" on the allocated type, and the nested
; types, but not on the pointed-to types.
%struct.test01a = type { i32*, %struct.test01b, %struct.test01d* }
%struct.test01b = type { i32, %struct.test01c }
%struct.test01c = type { i32 }
%struct.test01d = type { i32, %struct.test01e }
%struct.test01e = type { i32 }
define void @test01() {
  %local = alloca %struct.test01a
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Contains nested structure | Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Nested structure | Contains nested structure | Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test01b

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01c
; CHECK: Safety data: Nested structure | Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test01c

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01d
; CHECK: Safety data: Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01d

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01e
; CHECK: Safety data: Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01e


; Allocation of array of structures should be "Local instance"
%struct.test02 = type { i32, i32 }
define void @test02() {
  %local = alloca [4 x %struct.test02]
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test02


!1 = !{i32 0, i32 1}  ; i32*
!2 = !{%struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!3 = !{%struct.test01d zeroinitializer, i32 1}  ; %struct.test01d*
!4 = !{i32 0, i32 0}  ; i32
!5 = !{%struct.test01c zeroinitializer, i32 0}  ; %struct.test01c
!6 = !{%struct.test01e zeroinitializer, i32 0}  ; %struct.test01e
!7 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !2, !3} ; { i32*, %struct.test01b, %struct.test01d* }
!8 = !{!"S", %struct.test01b zeroinitializer, i32 2, !4, !5} ; { i32, %struct.test01c }
!9 = !{!"S", %struct.test01c zeroinitializer, i32 1, !4} ; { i32 }
!10 = !{!"S", %struct.test01d zeroinitializer, i32 2, !4, !6} ; { i32, %struct.test01e }
!11 = !{!"S", %struct.test01e zeroinitializer, i32 1, !4} ; { i32 }
!12 = !{!"S", %struct.test02 zeroinitializer, i32 2, !4, !4} ; { i32, i32 }

!intel.dtrans.types = !{!7, !8, !9, !10, !11, !12}
