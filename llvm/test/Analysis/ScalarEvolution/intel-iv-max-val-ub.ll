; RUN: opt < %s -analyze -enable-new-pm=0 -scalar-evolution | FileCheck %s
; RUN: opt < %s -disable-output "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s

; Verify that the range of IV phi was refined to exclude max signed value as
; that would result in undefined behavior. This is because IV increment %inc
; which is marked as <nsw> and used in the backedge comparison becomes poison
; for this case.

; CHECK:  %i.06 = phi i32 [ %inc, %for.body ], [ 1, %entry ]
; CHECK-NEXT:  -->  {1,+,1}<nuw><nsw><%for.body> U: [1,2147483647) S: [1,2147483647)


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* nocapture %A, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp5 = icmp slt i32 %n, 1
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

