; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test safety analysis for a memcpy call that is not a structure type to ensure
; the analysis collects the type info objects for the array types.

@buf = internal global [512 x i8] zeroinitializer
@digit = private unnamed_addr constant [11 x i8] c"0123456789\00"

define void @test() {
  %dst = getelementptr [512 x i8], [512 x i8]* @buf, i64 0, i64 0
  %src = getelementptr [11 x i8], [11 x i8]* @digit, i64 0, i64 0
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %dst, i8* %src, i64 11, i1 false)
  ret void
}

declare !intel.dtrans.func.type !2 void @llvm.memcpy.p0i8.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2", i64, i1)

!1 = !{i8 0, i32 1}  ; i8*
!2 = distinct !{!1, !1}

!intel.dtrans.types = !{}

; CHECK: DTRANS_ArrayInfo:
; CHECK:   LLVMType: [11 x i8]

; CHECK: DTRANS_ArrayInfo:
; CHECK:   LLVMType: [512 x i8]
