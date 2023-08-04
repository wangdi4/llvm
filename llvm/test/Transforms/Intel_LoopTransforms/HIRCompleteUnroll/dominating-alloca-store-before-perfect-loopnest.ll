; REQUIRES: asserts

; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,print<hir>,hir-pre-vec-complete-unroll" -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that we conclude that load of (%Tmp1)[0][i1] and (%Tmp2)[0][i1] can be
; eliminated after unrolling the two innermost loops by searching for dominating
; store before the outer loops which form perfect loopnest.

; CHECK: BEGIN REGION { }
; CHECK: (%Tmp1)[0][0] = 5;
; CHECK: (%Tmp1)[0][1] = 5;
; CHECK: (%Tmp2)[0][0] = 50;
; CHECK: (%Tmp2)[0][1] = 100;

; CHECK: + DO i1 = 0, 249, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 1, 1   <DO_LOOP>
; CHECK: |   |   %t2 = (%Tmp1)[0][i2];
; CHECK: |   |   %t4 = (%A)[i1 + i2];
; CHECK: |   |   (%A)[i1 + i2] = %t2 + %t4;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; CHECK: + DO i1 = 0, 249, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 1, 1   <DO_LOOP>
; CHECK: |   |   %t5 = (%Tmp2)[0][i2];
; CHECK: |   |   %t7 = (%B)[i1 + i2];
; CHECK: |   |   (%B)[i1 + i2] = %t5 + %t7;
; CHECK: |   + END LOOP
; CHECK: + END LOOP

; CHECK: ret ;
; CHECK: END REGION


; Number is obtained by multiplying with the trip count.

; CHECK: Number of memrefs which can be eliminated: 2
; CHECK: DO i2

; CHECK: Number of memrefs which can be eliminated: 2
; CHECK: DO i2


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly nofree nosync nounwind uwtable
define dso_local void @foo(ptr nocapture noundef %A, ptr nocapture noundef %B) local_unnamed_addr #0 {
entry:
  %Tmp1 = alloca [2 x i32], align 4
  %Tmp2 = alloca [2 x i32], align 4
  br label %bb

bb:
  store i32 5, ptr %Tmp1, align 4
  %arrayidx1 = getelementptr inbounds [2 x i32], ptr %Tmp1, i64 0, i64 1
  store i32 5, ptr %arrayidx1, align 4
  store i32 50, ptr %Tmp2, align 4
  %arrayidx3 = getelementptr inbounds [2 x i32], ptr %Tmp2, i64 0, i64 1
  store i32 100, ptr %arrayidx3, align 4
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %entry, %for.inc11
  %indvars.iv51 = phi i64 [ 0, %bb ], [ %indvars.iv.next52, %for.inc11 ]
  br label %for.body6

for.body6:                                        ; preds = %for.cond4.preheader, %for.body6
  %cmp5 = phi i1 [ true, %for.cond4.preheader ], [ false, %for.body6 ]
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ 1, %for.body6 ]
  %arrayidx7 = getelementptr inbounds [2 x i32], ptr %Tmp1, i64 0, i64 %indvars.iv
  %t2 = load i32, ptr %arrayidx7, align 4
  %t3 = add nuw nsw i64 %indvars.iv, %indvars.iv51
  %arrayidx9 = getelementptr inbounds i32, ptr %A, i64 %t3
  %t4 = load i32, ptr %arrayidx9, align 4
  %add10 = add nsw i32 %t4, %t2
  store i32 %add10, ptr %arrayidx9, align 4
  br i1 %cmp5, label %for.body6, label %for.inc11

for.inc11:                                        ; preds = %for.body6
  %indvars.iv.next52 = add nuw nsw i64 %indvars.iv51, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next52, 250
  br i1 %exitcond.not, label %for.cond17.preheader.preheader, label %for.cond4.preheader

for.cond17.preheader.preheader:                   ; preds = %for.inc11
  br label %for.cond17.preheader

for.cond17.preheader:                             ; preds = %for.cond17.preheader.preheader, %for.inc29
  %indvars.iv56 = phi i64 [ %indvars.iv.next57, %for.inc29 ], [ 0, %for.cond17.preheader.preheader ]
  br label %for.body19

for.body19:                                       ; preds = %for.cond17.preheader, %for.body19
  %cmp18 = phi i1 [ true, %for.cond17.preheader ], [ false, %for.body19 ]
  %indvars.iv53 = phi i64 [ 0, %for.cond17.preheader ], [ 1, %for.body19 ]
  %arrayidx21 = getelementptr inbounds [2 x i32], ptr %Tmp2, i64 0, i64 %indvars.iv53
  %t5 = load i32, ptr %arrayidx21, align 4
  %t6 = add nuw nsw i64 %indvars.iv53, %indvars.iv56
  %arrayidx24 = getelementptr inbounds i32, ptr %B, i64 %t6
  %t7 = load i32, ptr %arrayidx24, align 4
  %add25 = add nsw i32 %t7, %t5
  store i32 %add25, ptr %arrayidx24, align 4
  br i1 %cmp18, label %for.body19, label %for.inc29

for.inc29:                                        ; preds = %for.body19
  %indvars.iv.next57 = add nuw nsw i64 %indvars.iv56, 1
  %exitcond58.not = icmp eq i64 %indvars.iv.next57, 250
  br i1 %exitcond58.not, label %for.end31, label %for.cond17.preheader

for.end31:                                        ; preds = %for.inc29
  ret void
}

