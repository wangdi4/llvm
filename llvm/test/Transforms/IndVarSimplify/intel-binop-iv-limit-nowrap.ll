; RUN: opt -passes="loop(indvars)" < %s -S | FileCheck %s

; Verify that nsw flag was added to new iv limit (%n + 3) which is generated
; by adding 1 to original iv limit of (%n + 2).

; CHECK: [[IVLIMIT:%.*]] = add nsw i32 %n, 3
; CHECK: %wide.trip.count = sext i32 [[IVLIMIT]] to i64

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* nocapture %p, i32 %n) local_unnamed_addr #0 {
entry:
  %add = add nsw i32 %n, 2
  %cmp5 = icmp slt i32 %add, 1
  br i1 %cmp5, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.06 = phi i32 [ %inc, %for.body ], [ 1, %for.body.preheader ]
  %idxprom = zext i32 %i.06 to i64
  %ptridx = getelementptr inbounds i32, i32* %p, i64 %idxprom
  store i32 %i.06, i32* %ptridx, align 4
  %inc = add nuw nsw i32 %i.06, 1
  %cmp = icmp sgt i32 %inc, %add
  br i1 %cmp, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

