; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Basic test of the DTransSafetyAnalyzer pass to check that TypeInfo
; objects get created and reported for the structure types. Also,
; checks that 'Contains nested structure' and 'Nested structure'
; safety bit get set. Also, checks printing of structure field types.

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

%struct.test01a0 = type { %struct.test01a1 }
%struct.test01a1 = type { [4 x %struct.test01a2] }
%struct.test01a2 = type { %struct.test01a2impl* }
%struct.test01a2impl = type { i32, %struct.test01a3*, %struct.test01a4* }
%struct.test01a3 = type { i16, i16 }
%struct.test01a4 = type { i64, [8 x i16] }
define void @test01() {
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a0
; CHECK: Field LLVM Type: %struct.test01a1
; CHECK: DTrans Type: %struct.test01a1
; CHECK: Safety data: Contains nested structure
; CHECK: End LLVMType: %struct.test01a0

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a1
; CHECK: Field LLVM Type: [4 x %struct.test01a2]
; CHECK: DTrans Type: [4 x %struct.test01a2]
; CHECK: Safety data: Nested structure | Contains nested structure
; CHECK: End LLVMType: %struct.test01a1

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a2
; CHECK-NONOPAQUE: Field LLVM Type: %struct.test01a2impl*
; CHECK-OPAQUE: Field LLVM Type: ptr
; CHECK: DTrans Type: %struct.test01a2impl*
; CHECK: Safety data: Nested structure
; CHECK: End LLVMType: %struct.test01a2

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a2impl
; CHECK: Field LLVM Type: i32
; CHECK: DTrans Type: i32
; CHECK-NONOPAQUE: Field LLVM Type: %struct.test01a3*
; CHECK-OPAQUE: Field LLVM Type: ptr
; CHECK: DTrans Type: %struct.test01a3*
; CHECK-NONOPAQUE: Field LLVM Type: %struct.test01a4*
; CHECK-OPAQUE: Field LLVM Type: ptr
; CHECK: DTrans Type: %struct.test01a4*
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01a2impl

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a3
; CHECK: Field LLVM Type: i16
; CHECK: DTrans Type: i16
; CHECK: Field LLVM Type: i16
; CHECK: DTrans Type: i16
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01a3

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a4
; CHECK: Field LLVM Type: i64
; CHECK: DTrans Type: i64
; CHECK: Field LLVM Type: [8 x i16]
; CHECK: DTrans Type: [8 x i16]
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01a4

; CHECK: DTRANS_ArrayInfo:
; CHECK: LLVMType: [4 x %struct.test01a2]
; CHECK: Number of elements: 4
; CHECK: Element DTrans Type: %struct.test01a2

; CHECK: DTRANS_ArrayInfo:
; CHECK: LLVMType: [8 x i16]
; CHECK: Number of elements: 8
; CHECK: Element DTrans Type: i16


!1 = !{%struct.test01a1 zeroinitializer, i32 0}  ; %struct.test01a1
!2 = !{!"A", i32 4, !3}  ; [4 x %struct.test01a2]
!3 = !{%struct.test01a2 zeroinitializer, i32 0}  ; %struct.test01a2
!4 = !{%struct.test01a2impl zeroinitializer, i32 1}  ; %struct.test01a2impl*
!5 = !{i32 0, i32 0}  ; i32
!6 = !{%struct.test01a3 zeroinitializer, i32 1}  ; %struct.test01a3*
!7 = !{%struct.test01a4 zeroinitializer, i32 1}  ; %struct.test01a4*
!8 = !{i16 0, i32 0}  ; i16
!9 = !{i64 0, i32 0}  ; i64
!10 = !{!"A", i32 8, !8}  ; [8 x i16]
!11 = !{!"S", %struct.test01a0 zeroinitializer, i32 1, !1} ; { %struct.test01a1 }
!12 = !{!"S", %struct.test01a1 zeroinitializer, i32 1, !2} ; { [4 x %struct.test01a2] }
!13 = !{!"S", %struct.test01a2 zeroinitializer, i32 1, !4} ; { %struct.test01a2impl* }
!14 = !{!"S", %struct.test01a2impl zeroinitializer, i32 3, !5, !6, !7} ; { i32, %struct.test01a3*, %struct.test01a4* }
!15 = !{!"S", %struct.test01a3 zeroinitializer, i32 2, !8, !8} ; { i16, i16 }
!16 = !{!"S", %struct.test01a4 zeroinitializer, i32 2, !9, !10} ; { i64, [8 x i16] }

!intel.dtrans.types = !{!11, !12, !13, !14, !15, !16}
