; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-unroll-and-jam,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that unroll & jam gives up in the presence of unsafe call @bar(i1).

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK: |   + DO i2 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK: |   |   %call.us = @bar(i1);
; CHECK: |   |   %ld = (%A)[i2];
; CHECK: |   |   (%A)[i2] = %call.us + %ld;
; CHECK: |   + END LOOP
; CHECK: + END LOOP
; CHECK: END REGION


source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(ptr nocapture %A, i32 %n) {
entry:
  %cmp16 = icmp sgt i32 %n, 0
  br i1 %cmp16, label %for.cond1.preheader.us.preheader, label %for.end6

for.cond1.preheader.us.preheader:                 ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.cond1.preheader.us

for.cond1.preheader.us:                           ; preds = %for.cond1.for.inc4_crit_edge.us, %for.cond1.preheader.us.preheader
  %i.017.us = phi i32 [ %inc5.us, %for.cond1.for.inc4_crit_edge.us ], [ 0, %for.cond1.preheader.us.preheader ]
  br label %for.body3.us

for.body3.us:                                     ; preds = %for.body3.us, %for.cond1.preheader.us
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader.us ], [ %indvars.iv.next, %for.body3.us ]
  %call.us = tail call i32 @bar(i32 %i.017.us) #2
  %arrayidx.us = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %ld = load i32, ptr %arrayidx.us, align 4
  %add.us = add nsw i32 %ld, %call.us
  store i32 %add.us, ptr %arrayidx.us, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond1.for.inc4_crit_edge.us, label %for.body3.us

for.cond1.for.inc4_crit_edge.us:                  ; preds = %for.body3.us
  %inc5.us = add nuw nsw i32 %i.017.us, 1
  %exitcond18 = icmp eq i32 %inc5.us, %n
  br i1 %exitcond18, label %for.end6.loopexit, label %for.cond1.preheader.us

for.end6.loopexit:                                ; preds = %for.cond1.for.inc4_crit_edge.us
  br label %for.end6

for.end6:                                         ; preds = %for.end6.loopexit, %entry
  ret void
}

declare dso_local i32 @bar(i32) local_unnamed_addr #1

