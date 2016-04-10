; RUN: opt < %s -loop-simplify | opt -analyze -hir-scc-formation | FileCheck %s

; Check that the SCC with the type mismatch is removed (k.024 -> %.add -> %j.0 -> %conv10) so we don't form any SCCs.
; CHECK-NOT: SCC1



; ModuleID = 'scc-type-mismatch.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i16* nocapture %A, i32 %n) {
entry:
  %cmp.22 = icmp sgt i32 %n, 0
  br i1 %cmp.22, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %if.end.7
  %indvars.iv = phi i64 [ %indvars.iv.next, %if.end.7 ], [ 0, %entry ]
  %k.024 = phi i16 [ %conv10, %if.end.7 ], [ 0, %entry ]
  %conv = sext i16 %k.024 to i32
  %0 = trunc i64 %indvars.iv to i32
  %add = add nsw i32 %conv, %0
  %arrayidx = getelementptr inbounds i16, i16* %A, i64 %indvars.iv
  %1 = load i16, i16* %arrayidx, align 2
  %cmp2 = icmp sgt i16 %1, 5
  br i1 %cmp2, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %inc = add nsw i32 %add, 1
  br label %if.end.7

if.else:                                          ; preds = %for.body
  %cmp4 = icmp slt i32 %add, 10
  %.add = select i1 %cmp4, i32 4, i32 %add
  br label %if.end.7

if.end.7:                                         ; preds = %if.else, %if.then
  %j.0 = phi i32 [ %inc, %if.then ], [ %.add, %if.else ]
  store i16 %k.024, i16* %arrayidx, align 2
  %conv10 = trunc i32 %j.0 to i16
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %if.end.7, %entry
  ret void
}

