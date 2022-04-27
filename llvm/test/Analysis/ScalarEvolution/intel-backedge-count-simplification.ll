; RUN: opt < %s -analyze -enable-new-pm=0 -scalar-evolution | FileCheck %s
; RUN: opt < %s -disable-output "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s

; Verify that the smax expr in backedge taken count of the loop is simplified.
; Original the expr was like this-
; (-1 + ((1 + (-1 * %div26)<nsw>)<nsw> smax (1 + %div26)<nsw>) + %div26)

; CHECK: Loop %for.body: backedge-taken count is (2 * %div26)<nsw>

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* nocapture %A, i32 %n, i32 %lb) {
entry:
  %div26 = sdiv i32 %n, 2
  %sub27 = sub nsw i32 0, %div26
  %cmp5 = icmp slt i32 %div26, %sub27
  br i1 %cmp5, label %for.end, label %for.body

for.body:                                         ; preds = %entry, %for.body
  %i.06 = phi i32 [ %inc, %for.body ], [ %sub27, %entry ]
  %idxprom = zext i32 %i.06 to i64
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %idxprom
  store i32 %i.06, i32* %arrayidx, align 4
  %inc = add nuw nsw i32 %i.06, 1
  %cmp = icmp sgt i32 %inc, %div26
  br i1 %cmp, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

