; REQUIRES: asserts

; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,print<hir>,hir-pre-vec-complete-unroll" -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that we conclude that load (%Tmp1)[0][i1] can be eliminated after
; unrolling by searching for dominating store before the loop.

; CHECK: BEGIN REGION { }
; CHECK: (%Tmp1)[0][0] = 5;
; CHECK: (%Tmp1)[0][1] = 10;

; CHECK: + DO i1 = 0, 1, 1   <DO_LOOP>
; CHECK: |   %t1 = (%Tmp1)[0][i1];
; CHECK: |   %t2 = (%A)[i1];
; CHECK: |   (%A)[i1] = %t1 + %t2;
; CHECK: + END LOOP

; CHECK: ret ;
; CHECK: END REGION


; Number is obtained by multiplying with the trip count.

; CHECK: Number of memrefs which can be eliminated: 2


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly nofree nosync nounwind uwtable
define dso_local void @foo(ptr nocapture noundef %A) local_unnamed_addr #0 {
entry:
  %Tmp1 = alloca [2 x i32], align 4
  br label %bb

bb:
  store i32 5, ptr %Tmp1, align 4
  %arrayidx1 = getelementptr inbounds [2 x i32], ptr %Tmp1, i64 0, i64 1
  store i32 10, ptr %arrayidx1, align 4
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %cmp = phi i1 [ true, %bb ], [ false, %for.body ]
  %indvars.iv = phi i64 [ 0, %bb ], [ 1, %for.body ]
  %arrayidx2 = getelementptr inbounds [2 x i32], ptr %Tmp1, i64 0, i64 %indvars.iv
  %t1 = load i32, ptr %arrayidx2, align 4
  %arrayidx4 = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %t2 = load i32, ptr %arrayidx4, align 4
  %add = add nsw i32 %t2, %t1
  store i32 %add, ptr %arrayidx4, align 4
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  ret void
}

