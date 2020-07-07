; RUN: opt < %s -instcombine -S | FileCheck %s

; Verify that the backedge comparison is not modified by instconmbine in the
; presence of "pre_loopopt" attribute. This helps ScalarEvolution deduce that
; if %n is signed max, the behavior is undefined.

; CHECK-LABEL: @foo
; CHECK: [[CMP_NOT:%.*]] icmp sgt i32 %inc, %n

define dso_local void @foo(i32* nocapture %A, i32 %n) "pre_loopopt" {
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

; CHECK-LABEL: @bar
; CHECK: [[CMP_NOT:%.*]] = icmp slt i32 %i.06, %n

define dso_local void @bar(i32* nocapture %A, i32 %n) {
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
