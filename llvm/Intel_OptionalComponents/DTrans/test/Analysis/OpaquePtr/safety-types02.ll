; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; Basic test of the DTransSafetyAnalyzer pass collection of
; structure properties. Also, checks printing of structure field types.

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

%struct.base = type { i32 (...)** }
%struct.derived1 = type { %struct.base, i8 }
%struct.derived2 = type { %struct.derived1, i32 }
%struct.empty = type { }
%struct.fptrs = type { void (i32, i64*, i64)*, i32, i32 }
%struct.zeroarray = type { i32, [0 x i32] }
define void @test01() {
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.base
; CHECK-NONOPAQUE: Field LLVM Type: i32 (...)**
; CHECK-OPAQUE: Field LLVM Type: ptr
; CHECK: DTrans Type: i32 (...)**
; CHECK: Safety data: Nested structure | Has vtable
; CHECK: End LLVMType: %struct.base

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.derived1
; CHECK: Field LLVM Type: %struct.base
; CHECK: DTrans Type: %struct.base
; CHECK: Field LLVM Type: i8
; CHECK: DTrans Type: i8
; CHECK: Safety data: Nested structure | Contains nested structure
; CHECK: End LLVMType: %struct.derived1

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.derived2
; CHECK: Field LLVM Type: %struct.derived1
; CHECK: DTrans Type: %struct.derived1
; CHECK: Field LLVM Type: i32
; CHECK: DTrans Type: i32
; CHECK: Safety data: Contains nested structure
; CHECK: End LLVMType: %struct.derived2

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.empty
; CHECK: Safety data: No fields in structure
; CHECK: End LLVMType: %struct.empty

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.fptrs
; CHECK-NONOPAQUE: Field LLVM Type: void (i32, i64*, i64)*
; CHECK-OPAQUE: Field LLVM Type: ptr
; CHECK: DTrans Type: void (i32, i64*, i64)*
; CHECK: Field LLVM Type: i32
; CHECK: DTrans Type: i32
; CHECK: Field LLVM Type: i32
; CHECK: DTrans Type: i32
; CHECK: Safety data: Has function ptr
; CHECK: End LLVMType: %struct.fptrs

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.zeroarray
; CHECK: Field LLVM Type: i32
; CHECK: DTrans Type: i32
; CHECK: Field LLVM Type: [0 x i32]
; CHECK: DTrans Type: [0 x i32]
; CHECK: Safety data: Has zero-sized array
; CHECK: End LLVMType: %struct.zeroarray


!1 = !{!"F", i1 true, i32 0, !2}  ; i32 (...)
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!1, i32 2}  ; i32 (...)**
!4 = !{%struct.base zeroinitializer, i32 0}  ; %struct.base
!5 = !{i8 0, i32 0}  ; i8
!6 = !{%struct.derived1 zeroinitializer, i32 0}  ; %struct.derived1
!7 = !{!"F", i1 false, i32 3, !8, !2, !9, !10}  ; void (i32, i64*, i64)
!8 = !{!"void", i32 0}  ; void
!9 = !{i64 0, i32 1}  ; i64*
!10 = !{i64 0, i32 0}  ; i64
!11 = !{!7, i32 1}  ; void (i32, i64*, i64)*
!12 = !{!"A", i32 0, !2}  ; [0 x i32]
!13 = !{!"S", %struct.base zeroinitializer, i32 1, !3} ; { i32 (...)** }
!14 = !{!"S", %struct.derived1 zeroinitializer, i32 2, !4, !5} ; { %struct.base, i8 }
!15 = !{!"S", %struct.derived2 zeroinitializer, i32 2, !6, !2} ; { %struct.derived1, i32 }
!16 = !{!"S", %struct.empty zeroinitializer, i32 0} ; { }
!17 = !{!"S", %struct.fptrs zeroinitializer, i32 3, !11, !2, !2} ; { void (i32, i64*, i64)*, i32, i32 }
!18 = !{!"S", %struct.zeroarray zeroinitializer, i32 2, !2, !12} ; { i32, [0 x i32] }

!intel.dtrans.types = !{!13, !14, !15, !16, !17, !18}
