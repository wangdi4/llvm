; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -passes="dtrans-deletefield" -debug-only=dtrans-deletefield -dtrans-print-types -S 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Check that no unused fields are removed from a dope vector.

; Check the trace

; CHECK: DTRANS_StructInfo:
; CHECK:  LLVMType: %struct.dvty = type { i64*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
; CHECK: Name: struct.dvty
; CHECK: Number of fields: 7
; CHECK: Safety data:{{.*}}Dope vector
; CHECK: DTRANS_StructInfo:
; CHECK: Delete field: looking for candidate structures.
; CHECK: LLVM Type: %struct.dvty
; CHECK: Can delete field: %struct.dvty @ 1
; CHECK: Can delete field: %struct.dvty @ 2
; CHECK: Can delete field: %struct.dvty @ 3
; CHECK: Can delete field: %struct.dvty @ 4
; CHECK: Can delete field: %struct.dvty @ 5
; CHECK: Rejecting %struct.dvty based on safety data.
; CHECK: No candidates found.

; Check the IR

; CHECK: %struct.dvty = type { i64*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
; CHECK: define internal i64 @foo(%struct.dvty* %arg0) {
; CHECK: %t1 = getelementptr inbounds %struct.dvty, %struct.dvty* %arg0, i64 0, i32 6, i64 0
; CHECK: %t2 = getelementptr inbounds %struct.dvty, %struct.dvty* %arg0, i64 0, i32 0

declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32)

%struct.dvty = type { i64*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

define internal i64 @foo(%struct.dvty* %arg0) {
  %t1 = getelementptr inbounds %struct.dvty, %struct.dvty* %arg0, i64 0, i32 6, i64 0
  %t2 = getelementptr inbounds %struct.dvty, %struct.dvty* %arg0, i64 0, i32 0
  %t3 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %t1, i64 0, i32 0
  %t4 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %t1, i64 0, i32 1
  %t5 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %t3, i32 0)
  %t6 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %t4, i32 0)
  %t7 = load i64, i64* %t5, align 1
  %t8 = load i64*, i64** %t2, align 1
  %t9 = load i64, i64* %t6, align 1
  %t10 = load i64, i64* %t8, align 1
  %t11 = add nuw nsw i64 %t7, %t9
  %t12 = add nuw nsw i64 %t10, %t11
  ret i64 %t12;
}
