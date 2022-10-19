; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test of setting the "Global instance" safety flag.

%struct.test01a = type { i32*, %struct.test01b, %struct.test01d* }
%struct.test01b = type { i32, %struct.test01c }
%struct.test01c = type { i32 }
%struct.test01d = type { i32, %struct.test01e }
%struct.test01e = type { i32 }

@globalStruct = internal global %struct.test01a zeroinitializer

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Global instance | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Global instance | Nested structure | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01b

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01c
; CHECK: Safety data: Global instance | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01c

; TODO: Even though there is a pointer of this type in a global instantiation,
; this is not treated as having the "Global pointer" safety flag because
; a direct instantiation of a pointer to the type is not seen. This should
; be changed in the future to record that there is effectively a global
; pointer to this type.
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01d
; CHECK: Safety data: Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01d

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01e
; CHECK: Safety data: Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01e


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

!intel.dtrans.types = !{!7, !8, !9, !10, !11}
