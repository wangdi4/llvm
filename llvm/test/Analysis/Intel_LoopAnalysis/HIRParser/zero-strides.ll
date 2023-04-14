; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir>" -hir-create-function-level-region -disable-output  2>&1 | FileCheck %s

; Verify that we are able to successfully parse a subscript chain with zero
; strides in dimension 1 and 2. Previously, we were trying to divide by zero
; and failing.

; CHECK: %gep = &((%p)[0][0]);
; CHECK: %ld = (%gep)[0][0];
; CHECK: ret %ld;


define i32 @foo(ptr %p) {
entry:
  br label %bb

bb:
  %gep = getelementptr inbounds [0 x i32], ptr %p, i64 0, i64 0
  %sub1 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %gep, i64 1)
  %sub2 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 0, ptr nonnull elementtype(i32) %sub1, i64 1)
  %sub3 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 0, ptr nonnull elementtype(i32) %sub2, i64 1)
  %ld = load i32, ptr %sub3
  ret i32 %ld
}

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)
