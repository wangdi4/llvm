; RUN: opt -enable-new-pm=0 -O2 -hir-print-after-all < %s 2>&1 | FileCheck %s
; RUN: opt -passes='default<O2>' -loopopt -hir-print-after-all -disable-output < %s 2>&1 | FileCheck %s

; CHECK: BEGIN REGION

define void @foo(i8* nocapture %p, i32 %n) "loopopt-pipeline"="full" {
entry:
  %cmp5 = icmp sgt i32 %n, 0
  br i1 %cmp5, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %conv = trunc i64 %indvars.iv to i8
  %arrayidx = getelementptr inbounds i8, i8* %p, i64 %indvars.iv
  store i8 %conv, i8* %arrayidx, align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}
