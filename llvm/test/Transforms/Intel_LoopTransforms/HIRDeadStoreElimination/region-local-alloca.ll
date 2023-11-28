; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that both stores to %A are eliminated by replacing them and the loads
; with temp since all uses of %A occur within region. The temps are propagated
; into their uses.

; CHECK: Dump Before

; CHECK: (%A)[0][5] = %t;
; CHECK: (%A)[0][1] = 10;
; CHECK: %ld1 = (%A)[0][5];
; CHECK: %ld2 = (%A)[0][1];
; CHECK: ret %ld1 + %ld2;

; CHECK: Dump After

; CHECK-NOT: %temp =
; CHECK-NOT: %ld1 = 
; CHECK: ret %t + 10;


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo(i32 %t) {
entry:
  %A = alloca [10 x i32], align 16
  %bc = bitcast ptr %A to ptr
  %gep = getelementptr inbounds [10 x i32], ptr %A, i64 0, i64 5
  %A0 = getelementptr inbounds [10 x i32], ptr %A, i64 0, i64 0
  %subs = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %A0, i64 2)
  br label %bb

bb:
  store i32 %t, ptr %gep, align 4
  store i32 10, ptr %subs, align 4
  %ld1 = load i32, ptr %gep, align 4
  %ld2 = load i32, ptr %subs, align 4
  %add = add i32 %ld1, %ld2
  ret i32 %add
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

attributes #0 = { nounwind readnone speculatable }
attributes #1 = { argmemonly mustprogress nocallback nofree nosync nounwind willreturn }

