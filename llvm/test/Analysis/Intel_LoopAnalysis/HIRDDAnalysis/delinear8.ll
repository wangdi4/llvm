; RUN: opt -hir-ssa-deconstruction -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0 <%s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output <%s 2>&1 | FileCheck %s

; Check that linear DD refs were successfully delinearized. 

; Function: foo1

;    BEGIN REGION { }
;         + DO i1 = 0, %n + -2, 1   <DO_LOOP>
;         |   + DO i2 = 0, %n + -2, 1   <DO_LOOP>
;         |   |   %0 = (%A)[%n * i1 + i2 + %n + %k + 1];
;         |   |   (%A)[%n * i1 + i2 + %n + %k + 1] = %0 + 1;
;         |   + END LOOP
;         + END LOOP
;    END REGION

; CHECK: DD graph for function foo1:
; CHECK: 9:11 (%A)[%n * i1 + i2 + %n + %k + 1] --> (%A)[%n * i1 + i2 + %n + %k + 1] ANTI (= =) (0 0)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @foo1(i64 %n, i32* nocapture %A, i64 %k) local_unnamed_addr {
entry:
  %cmp24 = icmp sgt i64 %n, 1
  br i1 %cmp24, label %for.cond3.preheader.us.preheader, label %for.cond.cleanup

for.cond3.preheader.us.preheader:                 ; preds = %entry
  br label %for.cond3.preheader.us

for.cond3.preheader.us:                           ; preds = %for.cond3.preheader.us.preheader, %for.cond3.for.cond.cleanup5_crit_edge.us
  %i1.025.us = phi i64 [ %inc10.us, %for.cond3.for.cond.cleanup5_crit_edge.us ], [ 1, %for.cond3.preheader.us.preheader ]
  %mul.us = mul nsw i64 %i1.025.us, %n
  %add.us = add i64 %mul.us, %k
  br label %for.body6.us

for.body6.us:                                     ; preds = %for.cond3.preheader.us, %for.body6.us
  %j2.023.us = phi i64 [ 1, %for.cond3.preheader.us ], [ %inc.us, %for.body6.us ]
  %add7.us = add i64 %add.us, %j2.023.us
  %arrayidx.us = getelementptr inbounds i32, i32* %A, i64 %add7.us
  %0 = load i32, i32* %arrayidx.us, align 4
  %add8.us = add nsw i32 %0, 1
  store i32 %add8.us, i32* %arrayidx.us, align 4
  %inc.us = add nuw nsw i64 %j2.023.us, 1
  %exitcond.not = icmp eq i64 %inc.us, %n
  br i1 %exitcond.not, label %for.cond3.for.cond.cleanup5_crit_edge.us, label %for.body6.us

for.cond3.for.cond.cleanup5_crit_edge.us:         ; preds = %for.body6.us
  %inc10.us = add nuw nsw i64 %i1.025.us, 1
  %exitcond26.not = icmp eq i64 %inc10.us, %n
  br i1 %exitcond26.not, label %for.cond.cleanup.loopexit, label %for.cond3.preheader.us

for.cond.cleanup.loopexit:                        ; preds = %for.cond3.for.cond.cleanup5_crit_edge.us
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void
}


; Check that non-linear dd refs are not delinearized. 

; Function: foo2
;   BEGIN REGION { }
;       + DO i1 = 0, %n + -2, 1   <DO_LOOP>
;       |   + DO i2 = 0, %n + -2, 1   <DO_LOOP>
;       |   |   %0 = (%p)[i2 + 1];
;       |   |   %1 = (%A)[%n * i1 + i2 + %n + %0 + 1];
;       |   |   (%A)[%n * i1 + i2 + %n + %0 + 1] = %1 + 1;
;       |   + END LOOP
;       + END LOOP
;   END REGION

; CHECK: DD graph for function foo2:
; CHECK: 11:13 (%A)[%n * i1 + i2 + %n + %0 + 1] --> (%A)[%n * i1 + i2 + %n + %0 + 1] ANTI (* *) (? ?)


; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @foo2(i64 %n, i32* nocapture %A, i64* nocapture readonly %p) local_unnamed_addr {
entry:
  %cmp26 = icmp sgt i64 %n, 1
  br i1 %cmp26, label %for.cond3.preheader.us.preheader, label %for.cond.cleanup

for.cond3.preheader.us.preheader:                 ; preds = %entry
  br label %for.cond3.preheader.us

for.cond3.preheader.us:                           ; preds = %for.cond3.preheader.us.preheader, %for.cond3.for.cond.cleanup5_crit_edge.us
  %i1.027.us = phi i64 [ %inc11.us, %for.cond3.for.cond.cleanup5_crit_edge.us ], [ 1, %for.cond3.preheader.us.preheader ]
  %mul.us = mul nsw i64 %i1.027.us, %n
  br label %for.body6.us

for.body6.us:                                     ; preds = %for.cond3.preheader.us, %for.body6.us
  %j2.025.us = phi i64 [ 1, %for.cond3.preheader.us ], [ %inc.us, %for.body6.us ]
  %add.us = add nsw i64 %j2.025.us, %mul.us
  %arrayidx.us = getelementptr inbounds i64, i64* %p, i64 %j2.025.us
  %0 = load i64, i64* %arrayidx.us, align 8
  %add7.us = add nsw i64 %add.us, %0
  %arrayidx8.us = getelementptr inbounds i32, i32* %A, i64 %add7.us
  %1 = load i32, i32* %arrayidx8.us, align 4
  %add9.us = add nsw i32 %1, 1
  store i32 %add9.us, i32* %arrayidx8.us, align 4
  %inc.us = add nuw nsw i64 %j2.025.us, 1
  %exitcond.not = icmp eq i64 %inc.us, %n
  br i1 %exitcond.not, label %for.cond3.for.cond.cleanup5_crit_edge.us, label %for.body6.us

for.cond3.for.cond.cleanup5_crit_edge.us:         ; preds = %for.body6.us
  %inc11.us = add nuw nsw i64 %i1.027.us, 1
  %exitcond28.not = icmp eq i64 %inc11.us, %n
  br i1 %exitcond28.not, label %for.cond.cleanup.loopexit, label %for.cond3.preheader.us

for.cond.cleanup.loopexit:                        ; preds = %for.cond3.for.cond.cleanup5_crit_edge.us
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void
}

