; REQUIRES: asserts

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes="require<dtrans-safetyanalyzer>" -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test is to verify that the DTrans safety analyzer does not crash
; if the pointer type collected by the PtrTypeAnalyzer for the result
; type of a memory allocation is an opaque structure type.
; (CMPLRLLVM-47163)

%struct.unknown = type opaque
%struct.good = type { i32, ptr }

@mem = internal global %struct.good zeroinitializer

define void @test() {
  %p = tail call noalias ptr @malloc(i64 16)
  %addr = getelementptr %struct.good, ptr @mem, i64 0, i32 1
  store ptr %p, ptr %addr
  ret void
}

declare !intel.dtrans.func.type !6 noalias  "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

; CHECK: DTRANS_StructInfo:
; CHECK:   LLVMType: %struct.unknown = type opaque
; CHECK:   Safety data: Bad alloc size | No fields in structure{{ *}}
; CHECK:   End LLVMType: %struct.unknown = type opaque

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" }

!intel.dtrans.types = !{!1, !2}

!1 = !{!"S", %struct.unknown zeroinitializer, i32 -1} ; opaque
!2 = !{!"S", %struct.good zeroinitializer, i32 2, !3, !4} ; { i32, ; %struct.unknown* }
!3 = !{i32 0, i32 0}  ; i32
!4 = !{%struct.unknown zeroinitializer, i32 1} ; %struct.unknown*
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5}
