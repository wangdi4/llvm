; RUN: opt -passes="loop(indvars)" < %s -S | FileCheck %s

; Verify that nsw flag was added to new iv limit (%n + 1).

; CHECK: [[IVLIMIT:%.*]] = add nsw i32 %n, 1
; CHECK: br{{.*}}for.body

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* nocapture %A, i32 %s, i32 %n) {
entry:
  %cmp5 = icmp sgt i32 %s, %n
  br i1 %cmp5, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.06 = phi i32 [ %inc, %for.body ], [ %s, %for.body.preheader ]
  %idxprom = sext i32 %i.06 to i64
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %idxprom
  store i32 %i.06, i32* %arrayidx, align 4
  %inc = add nsw i32 %i.06, 1
  %cmp = icmp sgt i32 %inc, %n
  br i1 %cmp, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

