; RUN: opt < %s -analyze -enable-new-pm=0 -scalar-evolution | FileCheck %s
; RUN: opt < %s -disable-output "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s

; Verify that the smin expr in backedge taken count of the loop is simplified.
; Original expr was like this-
; ((zext i32 %t to i64) + (-1 * (1 smin (zext i32 %t to i64)))<nuw><nsw>)<nsw>

; CHECK: backedge-taken count is (-1 + (zext i32 %t to i64))<nsw>

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32 %t, i32* nocapture %A) {
entry:
  %cmp6 = icmp sgt i32 %t, 0
  br i1 %cmp6, label %while.body.preheader, label %while.end

while.body.preheader:                             ; preds = %entry
  %0 = zext i32 %t to i64
  br label %while.body

while.body:                                       ; preds = %while.body.preheader, %while.body
  %indvars.iv = phi i64 [ %0, %while.body.preheader ], [ %indvars.iv.next, %while.body ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, i32* %arrayidx, align 4
  %cmp = icmp sgt i64 %indvars.iv, 1
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  br i1 %cmp, label %while.body, label %while.end.loopexit

while.end.loopexit:                               ; preds = %while.body
  br label %while.end

while.end:                                        ; preds = %while.end.loopexit, %entry
  ret void
}

