; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>" -aa-pipeline="basic-aa" -S -disable-output  < %s 2>&1 | FileCheck %s

; Test checks that Opt Var Predicate cannot handle non-constant RHS in presence of
; constant in LHS of condition due to the absence of ext() on RHS of condition.

; HIR before optimization
;            BEGIN REGION { }
;                  + DO i64 i1 = 0, zext.i32.i64(%n) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 99>  <LEGAL_MAX_TC = 2147483646>
;                  |   %0 = (@B)[0][i1 + 1];
;                  |   (@A)[0][i1 + 1] = %0 + 1;
;                  |
;                  |   if (i1 + 1 > %m)
;                  |   <RVAL-REG> LINEAR i64 i1 + 1 {sb:2}
;                  |   <RVAL-REG> LINEAR i64 %m {sb:12}
;                  |   {
;                  |      (@C)[0][i1 + 1] = %0 + -1;
;                  |   }
;                  |   else
;                  |   {
;                  |      %1 = (@C)[0][i1 + 1];
;                  |      (@C)[0][i1 + 1] = %0 + %1;
;                  |   }
;                  + END LOOP
;            END REGION

; No optimization expected.
; CHECK: BEGIN REGION { }


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@A = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@C = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @foo(i32 noundef %n, i64 noundef %m) local_unnamed_addr {
entry:
  %cmp30 = icmp sgt i32 %n, 1
  br i1 %cmp30, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 1, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @B, i64 0, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %add = add nsw i32 %0, 1
  %arrayidx2 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %indvars.iv
  store i32 %add, ptr %arrayidx2, align 4
  %cmp3 = icmp sgt i64 %indvars.iv, %m
  br i1 %cmp3, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %sub = add nsw i32 %0, -1
  %arrayidx8 = getelementptr inbounds [100 x i32], ptr @C, i64 0, i64 %indvars.iv
  store i32 %sub, ptr %arrayidx8, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body
  %arrayidx12 = getelementptr inbounds [100 x i32], ptr @C, i64 0, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx12, align 4
  %add13 = add nsw i32 %1, %0
  store i32 %add13, ptr %arrayidx12, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %div = sdiv i32 %n, 2
  %idxprom14 = sext i32 %div to i64
  %arrayidx15 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %idxprom14
  %2 = load i32, ptr %arrayidx15, align 4
  %arrayidx18 = getelementptr inbounds [100 x i32], ptr @C, i64 0, i64 %idxprom14
  %3 = load i32, ptr %arrayidx18, align 4
  %add19 = add nsw i32 %3, %2
  ret i32 %add19
}

