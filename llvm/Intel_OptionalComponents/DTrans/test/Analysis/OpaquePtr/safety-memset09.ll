; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test safety analysis for a memset call that is not a structure type to ensure
; the analysis collects the type info objects for the array types.

@history_h = internal global [8 x [12 x [64 x i32]]] zeroinitializer

define void @test() {
  tail call void @llvm.memset.p0i8.i64(i8* bitcast ([8 x [12 x [64 x i32]]]* @history_h to i8*), i8 0, i64 24576, i1 false)
  ret void
}

declare !intel.dtrans.func.type !2 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8, i64, i1)

!1 = !{i8 0, i32 1}  ; i8*
!2 = distinct !{!1}

!intel.dtrans.types = !{}

; CHECK: DTRANS_ArrayInfo:
; CHECK:   LLVMType: [12 x [64 x i32]]

; CHECK: DTRANS_ArrayInfo:
; CHECK:   LLVMType: [64 x i32]

; CHECK: DTRANS_ArrayInfo:
; CHECK:   LLVMType: [8 x [12 x [64 x i32]]]
