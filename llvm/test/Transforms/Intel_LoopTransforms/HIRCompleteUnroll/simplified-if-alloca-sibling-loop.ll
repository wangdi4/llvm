; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-pre-vec-complete-unroll,print<hir>" -disable-hir-create-fusion-regions 2>&1 < %s | FileCheck %s

; Verify that both loops are unrolled before vectorizer.
; The first loop is unrolled on the optimisitic assumption that the simplifiable alloca store (%B)[0][i1] can be eliminated after unrolling by forwarding the RHS to its uses. The simplifiable if condition also helps.
; The second loop is unrolled because alloca load (%B)[0][i1] can be simplified because of the dominating simplifiable store (under a simplified if) in previous loop.

; CHECK: Function

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   if (i1 == 0)
; CHECK: |   {
; CHECK: |      %0 = (@A)[0][i1];
; CHECK: |      (%B)[0][i1] = %0 + 1;
; CHECK: |   }
; CHECK: |   else
; CHECK: |   {
; CHECK: |      (%B)[0][i1] = 5;
; CHECK: |   }
; CHECK: + END LOOP
; CHECK: END REGION

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   %1 = (%B)[0][i1];
; CHECK: |   %2 = (@A)[0][i1];
; CHECK: |   (%C)[0][i1] = %1 + %2;
; CHECK: + END LOOP
; CHECK: END REGION

; CHECK: Function

; CHECK-NOT: DO i1

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16

define i32 @foo() {
entry:
  %B = alloca [100 x i32], align 16
  %C = alloca [100 x i32], align 16
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.02 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp1 = icmp eq i32 %i.02, 0
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %idxprom = sext i32 %i.02 to i64
  %arrayidx = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %idxprom
  %0 = load i32, ptr %arrayidx, align 4
  %add = add nsw i32 %0, 1
  %idxprom2 = sext i32 %i.02 to i64
  %arrayidx3 = getelementptr inbounds [100 x i32], ptr %B, i64 0, i64 %idxprom2
  store i32 %add, ptr %arrayidx3, align 4
  br label %if.end

if.else:                                          ; preds = %for.body
  %idxprom4 = sext i32 %i.02 to i64
  %arrayidx5 = getelementptr inbounds [100 x i32], ptr %B, i64 0, i64 %idxprom4
  store i32 5, ptr %arrayidx5, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %inc = add nsw i32 %i.02, 1
  %cmp = icmp slt i32 %inc, 10
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  br label %for.body8

for.body8:                                        ; preds = %for.end, %for.inc16
  %i.11 = phi i32 [ 0, %for.end ], [ %inc17, %for.inc16 ]
  %idxprom9 = sext i32 %i.11 to i64
  %arrayidx10 = getelementptr inbounds [100 x i32], ptr %B, i64 0, i64 %idxprom9
  %1 = load i32, ptr %arrayidx10, align 4
  %idxprom11 = sext i32 %i.11 to i64
  %arrayidx12 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %idxprom11
  %2 = load i32, ptr %arrayidx12, align 4
  %add13 = add nsw i32 %1, %2
  %idxprom14 = sext i32 %i.11 to i64
  %arrayidx15 = getelementptr inbounds [100 x i32], ptr %C, i64 0, i64 %idxprom14
  store i32 %add13, ptr %arrayidx15, align 4
  br label %for.inc16

for.inc16:                                        ; preds = %for.body8
  %inc17 = add nsw i32 %i.11, 1
  %cmp7 = icmp slt i32 %inc17, 10
  br i1 %cmp7, label %for.body8, label %for.end18

for.end18:                                        ; preds = %for.inc16
  %arrayidx19 = getelementptr inbounds [100 x i32], ptr %C, i64 0, i64 2
  %3 = load i32, ptr %arrayidx19, align 8
  %arrayidx20 = getelementptr inbounds [100 x i32], ptr %C, i64 0, i64 3
  %4 = load i32, ptr %arrayidx20, align 4
  %add21 = add nsw i32 %3, %4
  ret i32 %add21
}
