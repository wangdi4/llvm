; RUN: opt -passes="instcombine" < %s -S | FileCheck %s

; Verify that %cmp5 in @foo is not optimized due to presence of "pre_loopopt"
; attribute.
; CHECK: @foo
; CHECK: %cmp5 = icmp sgt i32 %mul, 0

define dso_local void @foo(ptr nocapture %A, i32 %n) "pre_loopopt" {
entry:
  %mul = mul nsw i32 %n, 3
  %cmp5 = icmp sgt i32 %mul, 0
  br i1 %cmp5, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body
  %i.06 = phi i32 [ %inc, %for.body ], [ 0, %entry ]
  %conv = trunc i32 %i.06 to i8
  %idxprom = sext i32 %i.06 to i64
  %arrayidx = getelementptr inbounds i8, ptr %A, i64 %idxprom
  store i8 %conv, ptr %arrayidx, align 1
  %inc = add nsw i32 %i.06, 1
  %cmp = icmp slt i32 %inc, %mul
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body, %entry
  ret void
}

; Verify that %cmp5 in @foo1 is optimized.
; CHECK: @foo1
; CHECK: %cmp5 = icmp sgt i32 %n, 0


define dso_local void @foo1(ptr nocapture %A, i32 %n) {
entry:
  %mul = mul nsw i32 %n, 3
  %cmp5 = icmp sgt i32 %mul, 0
  br i1 %cmp5, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body
  %i.06 = phi i32 [ %inc, %for.body ], [ 0, %entry ]
  %conv = trunc i32 %i.06 to i8
  %idxprom = sext i32 %i.06 to i64
  %arrayidx = getelementptr inbounds i8, ptr %A, i64 %idxprom
  store i8 %conv, ptr %arrayidx, align 1
  %inc = add nsw i32 %i.06, 1
  %cmp = icmp slt i32 %inc, %mul
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body, %entry
  ret void
}
