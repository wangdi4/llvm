; RUN: opt -disable-output "-passes=print<scalar-evolution>" < %s 2>&1 | FileCheck %s

; Verify that the backedge taken count of the loop is simplified to-
; (-1 + (zext i32 %n to i64) + (sext i32 %n to i64))<nsw>

; Previously the loop entry guard wasn't being recognized and the backedge
; taken count was this with a smax in it-
; (-1 + (sext i32 %n to i64) + ((zext i32 %n to i64) smax (1 + (-1 * (sext i32 %n to i64))<nsw>)<nsw>))<nsw>

; CHECK: Loop %for.body: backedge-taken count is (-1 + (zext i32 %n to i64) + (sext i32 %n to i64))<nsw>

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32 noundef %n, ptr nocapture noundef writeonly %A) {
entry:
  %cmp5 = icmp sgt i32 %n, 0
  br i1 %cmp5, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %sub = sub nsw i32 0, %n
  %0 = sext i32 %sub to i64
  %1 = zext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %2 = trunc i64 %indvars.iv to i32
  store i32 %2, ptr %arrayidx, align 4
  %indvars.iv.next = add nsw i64 %indvars.iv, 1
  %cmp = icmp slt i64 %indvars.iv.next, %1
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}


