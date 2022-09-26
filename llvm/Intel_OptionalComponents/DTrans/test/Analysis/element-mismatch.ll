; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume  -dtransanalysis -dtrans-print-types -dtrans-outofboundsok=false -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes="require<dtransanalysis>" -dtrans-print-types -dtrans-outofboundsok=false -disable-output < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Verify that the assumption (DTransOutOfBoundsOK == false) relaxes
; the "Mismatched element access" safety check cascading effect.

; Test #0
; CHECK-LABEL: LLVMType: %struct.nested_0_0_t = type
; CHECK: Safety data:
; CHECK-NOT: Mismatched element access

; CHECK-LABEL: LLVMType: %struct.nested_0_1_t = type
; CHECK: Safety data:
; CHECK-NOT: Mismatched element access

; Test #1
; CHECK-LABEL: LLVMType: %struct.nested_1_0_t = type
; CHECK: Safety data:
; CHECK-SAME: Mismatched element access

; CHECK-LABEL: LLVMType: %struct.nested_1_1_t = type
; CHECK: Safety data:
; CHECK-NOT: Mismatched element access

; Test #0
; CHECK-LABEL: LLVMType: %struct.parent_0_t = type
; CHECK: Safety data:
; CHECK-SAME: Mismatched element access

; Test #1
; CHECK-LABEL: LLVMType: %struct.parent_1_t = type
; CHECK: Safety data:
; CHECK-SAME: Mismatched element access

%struct.parent_0_t = type { i8*, %struct.nested_0_0_t, %struct.nested_0_1_t }
%struct.nested_0_0_t = type { i32 }
%struct.nested_0_1_t = type { i32 }

define dso_local void @test_00(%struct.parent_0_t* %p) {
entry:
  %0 = bitcast %struct.parent_0_t* %p to i32**
  %1 = load i32*, i32** %0, align 8
  tail call void @foo(i32* %1)
  ret void
}

%struct.parent_1_t = type { i8*, %struct.nested_1_0_t, %struct.nested_1_1_t }
%struct.nested_1_0_t = type { i32 }
%struct.nested_1_1_t = type { i32 }

define dso_local void @test_01(%struct.parent_1_t* %p) {
entry:
  %np = getelementptr %struct.parent_1_t, %struct.parent_1_t* %p, i64 0, i32 1
  %0 = bitcast %struct.nested_1_0_t* %np to i32**
  %1 = load i32*, i32** %0, align 8
  tail call void @foo(i32* %1)
  ret void
}

declare dso_local void @foo(i32*)

