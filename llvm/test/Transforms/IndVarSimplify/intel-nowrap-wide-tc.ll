; RUN: opt -passes="loop(indvars)" < %s -S | FileCheck %s

; Verify that the generated add operation for wide trip count has nsw/nuw flag.

; CHECK: [[WIDE_TC:%.*]] = add nuw nsw i32 %n, 1
; CHECK-NEXT: %wide.trip.count = sext i32 [[WIDE_TC]] to i64


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* nocapture %A, i32 %n) {
entry:
  %cmp5 = icmp sgt i32 1, %n
  br i1 %cmp5, label %for.end, label %for.body

for.body:                                         ; preds = %entry, %for.body
  %i.06 = phi i32 [ %inc, %for.body ], [ 1, %entry ]
  %idxprom = zext i32 %i.06 to i64
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %idxprom
  store i32 %i.06, i32* %arrayidx, align 4
  %inc = add nuw nsw i32 %i.06, 1
  %cmp = icmp sgt i32 %inc, %n
  br i1 %cmp, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

