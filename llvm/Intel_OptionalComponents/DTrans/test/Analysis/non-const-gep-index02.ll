; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -dtransanalysis -whole-program-assume -dtrans-print-types -dtrans-outofboundsok=false -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="require<dtransanalysis>" -whole-program-assume -dtrans-print-types -dtrans-outofboundsok=false -disable-output < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Given a structure with an array field, check that access to that field with a non-constant offset doesn't lead to Bad pointer manipulation in the presence of isOutOfBoundsOK=false.

; Test1
; CHECK-LABEL: LLVMType: %struct.s1 = type
; CHECK: Safety data:
; CHECK-NOT: Bad pointer manipulation

%struct.s1 = type { i32, [10 x i8], i32 }
define dso_local void @test1(%struct.s1* %p, i64 %x) {
entry:
  %gep = getelementptr inbounds %struct.s1, %struct.s1* %p, i64 0, i32 1, i64 %x
  ret void
}

declare noalias i8* @malloc(i64)
