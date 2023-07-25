; RUN: opt < %s -analyze -bugpoint-enable-legacy-pm -scalar-evolution | FileCheck %s
; RUN: opt < %s -disable-output "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s

; Verify that we are able to compute and simplify (no smin) backedge taken count
; of the loop with unknown lower bound.

; CHECK: Loop %for.body: backedge-taken count is ((-1 * %n) + %lb)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr nocapture %A, i32 %n, i32 %lb) {
entry:
  %cmp5 = icmp sgt i32 %n, %lb
  br i1 %cmp5, label %for.end, label %for.body

for.body:                                         ; preds = %entry, %for.body
  %i.06 = phi i32 [ %inc, %for.body ], [ %lb, %entry ]
  %idxprom = zext i32 %i.06 to i64
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %idxprom
  store i32 %i.06, ptr %arrayidx, align 4
  %inc = add nuw nsw i32 %i.06, -1
  %cmp = icmp slt i32 %inc, %n
  br i1 %cmp, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

