; RUN: opt < %s -instcombine -S | FileCheck %s

; Verify that %cmp7 in @foo is not optimized due to presence of "pre_loopopt"
; attribute as it interferes with ztt recognition.
; CHECK: @foo
; CHECK: %cmp7 = icmp slt i32 %add, 1

define dso_local i32 @foo(i32 %n, i32* %p) "pre_loopopt" {
entry:
  %add = add nsw i32 %n, 2
  %cmp7 = icmp slt i32 %add, 1
  br i1 %cmp7, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body
  %t.09 = phi i32 [ %add1, %for.body ], [ 0, %entry ]
  %i.08 = phi i32 [ %inc, %for.body ], [ 1, %entry ]
  %add1 = add nuw nsw i32 %t.09, %i.08
  store i32 %add1, i32* %p, align 4
  %inc = add nuw nsw i32 %i.08, 1
  %cmp = icmp slt i32 %inc, %add
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body, %entry
  %t.0.lcssa = phi i32 [ 0, %entry ], [ %add1, %for.body ]
  ret i32 %t.0.lcssa
}


; Verify that %cmp7 in @foo1 is optimized.
; CHECK: @foo1
; CHECK: %cmp7 = icmp slt i32 %n, -1

define dso_local i32 @foo1(i32 %n, i32* %p) {
entry:
  %add = add nsw i32 %n, 2
  %cmp7 = icmp slt i32 %add, 1
  br i1 %cmp7, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body
  %t.09 = phi i32 [ %add1, %for.body ], [ 0, %entry ]
  %i.08 = phi i32 [ %inc, %for.body ], [ 1, %entry ]
  %add1 = add nuw nsw i32 %t.09, %i.08
  store i32 %add1, i32* %p, align 4
  %inc = add nuw nsw i32 %i.08, 1
  %cmp = icmp slt i32 %inc, %add
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body, %entry
  %t.0.lcssa = phi i32 [ 0, %entry ], [ %add1, %for.body ]
  ret i32 %t.0.lcssa
}
