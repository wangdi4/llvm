; RUN: opt < %s -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-array-section-analysis | FileCheck %s
; RUN: opt < %s -aa-pipeline=basic-aa -passes="hir-ssa-deconstruction,print<hir-array-section-analysis>" -disable-output 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, %m + -1, 1   <DO_LOOP>
;       |   + DO i2 = 0, 9, 1   <DO_LOOP>
;       |   |   (%p)[i2] = i2;
;       |   |   %0 = (%q)[i2][i2];
;       |   |   (%p)[i2 + 1] = %0 + 1;
;       |   + END LOOP
;       |
;       |
;       |   + DO i2 = 0, 8, 1   <DO_LOOP>
;       |   |   (%p)[i2 + 11] = i2 + 11;
;       |   |   %1 = (%q)[i2 + 11][i2 + 11];
;       |   |   (%p)[i2 + 12] = %1 + 1;
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { }
; CHECK:    + DO i1 = 0, %m + -1, 1   <DO_LOOP>
; CHECK-DAG:%p: (DEF) [i2,i2 + 1,i2 + 11,i2 + 12:0:20]
; CHECK-DAG:%q: (USE) [i2,i2 + 11:0:19][i2,i2 + 11:0:19]
; CHECK:    |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK-DAG:    %p: (DEF) [i2,i2 + 1:0:10]
; CHECK-DAG:    %q: (USE) [i2:0:9][i2:0:9]
; CHECK:    |   + END LOOP
; CHECK:    |   + DO i2 = 0, 8, 1   <DO_LOOP>
; CHECK-DAG:    %p: (DEF) [i2 + 11,i2 + 12:11:20]
; CHECK-DAG:    %q: (USE) [i2 + 11:11:19][i2 + 11:11:19]
; CHECK:    |   + END LOOP
; CHECK:    + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@p = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@q = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo(i32* noalias nocapture %p, [100 x i32]* nocapture readonly %q, i64 %n, i64 %m) local_unnamed_addr #0 {
entry:
  %cmp50 = icmp eq i64 %m, 0
  br i1 %cmp50, label %for.cond.cleanup, label %for.cond1.preheader.preheader

for.cond1.preheader.preheader:                    ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.cond.cleanup13
  %j.051 = phi i64 [ %inc26, %for.cond.cleanup13 ], [ 0, %for.cond1.preheader.preheader ]
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup13
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %i.048 = phi i64 [ 0, %for.cond1.preheader ], [ %add7, %for.body4 ]
  %conv = trunc i64 %i.048 to i32
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %i.048
  store i32 %conv, i32* %arrayidx, align 4
  %arrayidx6 = getelementptr inbounds [100 x i32], [100 x i32]* %q, i64 %i.048, i64 %i.048
  %0 = load i32, i32* %arrayidx6, align 4
  %add = add nsw i32 %0, 1
  %add7 = add nuw nsw i64 %i.048, 1
  %arrayidx8 = getelementptr inbounds i32, i32* %p, i64 %add7
  store i32 %add, i32* %arrayidx8, align 4
  %exitcond = icmp eq i64 %add7, 10
  br i1 %exitcond, label %for.body14.preheader, label %for.body4

for.body14.preheader:                             ; preds = %for.body4
  br label %for.body14

for.cond.cleanup13:                               ; preds = %for.body14
  %inc26 = add nuw i64 %j.051, 1
  %exitcond53 = icmp eq i64 %inc26, %m
  br i1 %exitcond53, label %for.cond.cleanup.loopexit, label %for.cond1.preheader

for.body14:                                       ; preds = %for.body14.preheader, %for.body14
  %i9.049 = phi i64 [ %add20, %for.body14 ], [ 11, %for.body14.preheader ]
  %conv15 = trunc i64 %i9.049 to i32
  %arrayidx16 = getelementptr inbounds i32, i32* %p, i64 %i9.049
  store i32 %conv15, i32* %arrayidx16, align 4
  %arrayidx18 = getelementptr inbounds [100 x i32], [100 x i32]* %q, i64 %i9.049, i64 %i9.049
  %1 = load i32, i32* %arrayidx18, align 4
  %add19 = add nsw i32 %1, 1
  %add20 = add nuw nsw i64 %i9.049, 1
  %arrayidx21 = getelementptr inbounds i32, i32* %p, i64 %add20
  store i32 %add19, i32* %arrayidx21, align 4
  %exitcond52 = icmp eq i64 %add20, 20
  br i1 %exitcond52, label %for.cond.cleanup13, label %for.body14
}

