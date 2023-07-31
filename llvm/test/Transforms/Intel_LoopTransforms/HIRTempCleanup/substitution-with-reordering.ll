; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>,hir-temp-cleanup,print<hir-framework>" -disable-output 2>&1 | FileCheck %s


; Verify that temp cleanup cleans up %best.014.out by moving its use in %tmp.015 before the invalidating definition %best.014.

; CHECK: Function:

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, sext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
; CHECK: |   %best.014.out = %best.014;
; CHECK: |   %0 = (@ordering)[0][i1];
; CHECK: |   %best.014 = (%0 > %best.014.out) ? %0 : %best.014;
; CHECK: |   %tmp.015 = (%0 > %best.014.out) ? i1 : %tmp.015;
; CHECK: + END LOOP
; CHECK: END REGION

; CHECK: Function:

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, sext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
; CHECK: |   %0 = (@ordering)[0][i1];
; CHECK: |   %tmp.015 = (%0 > %best.014) ? i1 : %tmp.015;
; CHECK: |   %best.014 = (%0 > %best.014) ? %0 : %best.014;
; CHECK: + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@ordering = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @maxloc(i32 %m) local_unnamed_addr {
entry:
  %cmp13 = icmp sgt i32 %m, 0
  br i1 %cmp13, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %m to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %tmp.015 = phi i32 [ 0, %for.body.preheader ], [ %spec.select12, %for.body ]
  %best.014 = phi i32 [ -111111111, %for.body.preheader ], [ %spec.select, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @ordering, i64 0, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %cmp1 = icmp sgt i32 %0, %best.014
  %spec.select = select i1 %cmp1, i32 %0, i32 %best.014
  %1 = trunc i64 %indvars.iv to i32
  %spec.select12 = select i1 %cmp1, i32 %1, i32 %tmp.015
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  %spec.select.lcssa = phi i32 [ %spec.select, %for.body ]
  %spec.select12.lcssa = phi i32 [ %spec.select12, %for.body ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %best.0.lcssa = phi i32 [ -111111111, %entry ], [ %spec.select.lcssa, %for.end.loopexit ]
  %tmp.0.lcssa = phi i32 [ 0, %entry ], [ %spec.select12.lcssa, %for.end.loopexit ]
  %add = add nsw i32 %tmp.0.lcssa, %best.0.lcssa
  ret i32 %add
}

