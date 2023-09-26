; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that we recognize load/store based on %A0 as alloca based, identify it
; as region local alloca and eliminate the store by propagating it into load.

; CHECK: Dump Before

; CHECK: (%A0)[0][1] = 10;
; CHECK: %ld = (%A0)[0][1];
; CHECK: ret %ld;

; CHECK: Dump After

; TODO: propgate region invariant rvals in copy propagation.

; CHECK: ret 10;


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo(i32 %t) {
entry:
  %A = alloca [10 x i32], align 16
  %A0 = getelementptr inbounds [10 x i32], ptr %A, i64 0, i64 0
  %subs1 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %A0, i64 2)
  %subs2 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20, ptr nonnull elementtype(i32) %subs1, i64 1)
  br label %bb

bb:
  store i32 10, ptr %subs2, align 4
  %ld = load i32, ptr %subs2, align 4
  ret i32 %ld
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

attributes #0 = { nounwind readnone speculatable }
