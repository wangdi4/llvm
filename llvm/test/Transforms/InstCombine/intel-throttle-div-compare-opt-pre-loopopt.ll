; RUN: opt < %s -instcombine -S | FileCheck %s

; Verify that %cmp6 in @foo is not optimized due to presence of "pre_loopopt"
; attribute.
; CHECK: @foo
; CHECK: %cmp6 = icmp sgt i64 %div, 0

source_filename = "div2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %A, i64 %n, i64 %m) #0 {
entry:
  %div = sdiv i64 %n, 2
  %cmp6 = icmp sgt i64 %div, 0
  br i1 %cmp6, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body
  %j.07 = phi i64 [ %inc, %for.body ], [ 0, %entry ]
  %conv = trunc i64 %j.07 to i32
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %j.07
  store i32 %conv, i32* %arrayidx, align 4
  %inc = add nuw nsw i64 %j.07, 1
  %exitcond = icmp eq i64 %inc, %div
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}


; Verify that %cmp6 in @foo1 is optimized. 
; CHECK: @foo1
; CHECK: %cmp6 = icmp sgt i64 %n, 1

; Function Attrs: norecurse nounwind uwtable
define void @foo1(i32* nocapture %A, i64 %n, i64 %m) {
entry:
  %div = sdiv i64 %n, 2
  %cmp6 = icmp sgt i64 %div, 0
  br i1 %cmp6, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body
  %j.07 = phi i64 [ %inc, %for.body ], [ 0, %entry ]
  %conv = trunc i64 %j.07 to i32
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %j.07
  store i32 %conv, i32* %arrayidx, align 4
  %inc = add nuw nsw i64 %j.07, 1
  %exitcond = icmp eq i64 %inc, %div
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

attributes #0 = { "pre_loopopt" }

