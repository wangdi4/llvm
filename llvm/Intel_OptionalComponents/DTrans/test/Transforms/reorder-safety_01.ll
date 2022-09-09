; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; This test verifies dtrans field reordering transformation doesn't select
; a structure as candidate due to safety conditions.

;  RUN: opt  < %s -whole-program-assume -dtrans-reorderfields -debug-only=dtrans-reorderfields -disable-output 2>&1 | FileCheck %s
;  RUN: opt  < %s -whole-program-assume -passes=dtrans-reorderfields -debug-only=dtrans-reorderfields -disable-output 2>&1 | FileCheck %s

; CHECK: Rejecting struct.test based on safety data


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

define i32 @main() {
entry:
  %call = tail call noalias i8* @calloc(i64 10, i64 48)
  %0 = bitcast i8* %call to %struct.test*
  tail call void @foo(%struct.test* %0)
  %h = getelementptr inbounds i8, i8* %call, i64 20
  %1 = bitcast i8* %h to i32*
  tail call void @foo1(i32* nonnull %1)
  ret i32 0
}

declare noalias i8* @calloc(i64, i64)

declare void @foo(%struct.test*)

declare void @foo1(i32*)
