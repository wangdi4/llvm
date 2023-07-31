; RUN: opt -hir-create-function-level-region -hir-details -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,print<hir-framework>,hir-dead-store-elimination,print<hir-framework>" 2>&1 < %s | FileCheck %s

; Verify that we are able to conclude post-domination for sibling loops with identical Ztt and eliminate the first loop as dead.

; Dump Before

; CHECK: Function: foo
; CHECK: + Ztt: if (%n > 0)
; CHECK: + DO i64 i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   (@A)[0][i1] = i1;
; CHECK: + END LOOP

; CHECK: + Ztt: if (%n > 0)
; CHECK: + DO i64 i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   (@A)[0][i1] = %m;
; CHECK: + END LOOP


; Dump After

; CHECK: Function: foo

; CHECK-NOT: DO

; CHECK: + Ztt: if (%n > 0)
; CHECK: + DO i64 i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   (@A)[0][i1] = %m;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable writeonly
define dso_local void @foo(i32 %n, i32 %m) local_unnamed_addr #0 {
entry:
  %cmp19 = icmp sgt i32 %n, 0
  %wide.trip.count2325 = zext i32 %n to i64
  br i1 %cmp19, label %for.body.preheader, label %for.cond1.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv21 = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next22, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %indvars.iv21
  %0 = trunc i64 %indvars.iv21 to i32
  store i32 %0, ptr %arrayidx, align 4
  %indvars.iv.next22 = add nuw nsw i64 %indvars.iv21, 1
  %exitcond24 = icmp eq i64 %indvars.iv.next22, %wide.trip.count2325
  br i1 %exitcond24, label %for.cond1.preheader.loopexit, label %for.body

for.cond1.preheader.loopexit:                     ; preds = %for.body
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.loopexit, %entry
  %cmp217 = icmp sgt i32 %n, 0
  br i1 %cmp217, label %for.body3.preheader, label %for.end8

for.body3.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.body3.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %indvars.iv
  store i32 %m, ptr %arrayidx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count2325
  br i1 %exitcond, label %for.end8.loopexit, label %for.body3

for.end8.loopexit:                                ; preds = %for.body3
  br label %for.end8

for.end8:                                         ; preds = %for.end8.loopexit, %for.cond1.preheader
  ret void
}

