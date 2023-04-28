; RUN: opt -disable-output "-passes=print<scalar-evolution>" < %s 2>&1 | FileCheck %s

; Verify that the backedge taken count of the loop is simplified to (2 * %n).
; Previously the loop entry guard wasn't being recognized and the backedge
; taken count was this with a smax in it-
; (((-1 * %n) smax %n) + %n)


; CHECK: Loop %for.body: backedge-taken count is (2 * %n)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i64 noundef %n, ptr nocapture noundef writeonly %A) {
entry:
  %cmp.not5 = icmp slt i64 %n, 0
  br i1 %cmp.not5, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  %sub = sub nsw i64 0, %n
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.06 = phi i64 [ %inc, %for.body ], [ %sub, %for.body.preheader ]
  %conv = trunc i64 %i.06 to i32
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %i.06
  store i32 %conv, ptr %arrayidx, align 4
  %inc = add nsw i64 %i.06, 1
  %cmp.not = icmp sgt i64 %inc, %n
  br i1 %cmp.not, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

