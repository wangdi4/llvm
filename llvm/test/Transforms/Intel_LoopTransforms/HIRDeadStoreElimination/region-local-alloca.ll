; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that both stores to %A are eliminated by replacing them and the loads
; with temp since all uses of %A occur within region.

; CHECK: Dump Before

; CHECK: (%A)[0][5] = %t;
; CHECK: (%A)[0][1] = 10;
; CHECK: %ld1 = (%A)[0][5];
; CHECK: %ld2 = (%A)[0][1];
; CHECK: ret %ld1 + %ld2;

; CHECK: Dump After

; TODO: propgate region invariant rvals in copy propagation.

; CHECK: %temp = %t;
; CHECK: %ld1 = %temp;
; CHECK: ret %ld1 + 10;


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo(i32 %t) {
entry:
  %A = alloca [10 x i32], align 16
  %bc = bitcast [10 x i32]* %A to i8*
  %gep = getelementptr inbounds [10 x i32], [10 x i32]* %A, i64 0, i64 5
  %A0 = getelementptr inbounds [10 x i32], [10 x i32]* %A, i64 0, i64 0
  %subs = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) %A0, i64 2)
  br label %bb

bb:
  store i32 %t, i32* %gep, align 4
  store i32 10, i32* %subs, align 4
  %ld1 = load i32, i32* %gep, align 4
  %ld2 = load i32, i32* %subs, align 4
  %add = add i32 %ld1, %ld2
  ret i32 %add
}

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #0

attributes #0 = { nounwind readnone speculatable }
attributes #1 = { argmemonly mustprogress nocallback nofree nosync nounwind willreturn }

