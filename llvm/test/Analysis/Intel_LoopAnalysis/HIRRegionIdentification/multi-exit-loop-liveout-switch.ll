; REQUIRES: asserts
; RUN: opt < %s -analyze -hir-region-identification -debug-only=hir-region-identification  2>&1 | FileCheck %s

; Verify that we skip switch which has multiple successors outside the loop.

; CHECK: Switch instruction with multiple successors outside the loop currently not supported.

define void @foo(i8* %call) {
entry:
  br label %land.lhs.true

land.lhs.true:                                    ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i8, i8* %call, i64 %indvars.iv
  %t3 = load i8, i8* %arrayidx, align 1
  switch i8 %t3, label %for.body [
    i8 0, label %for.end
    i8 32, label %for.end
    i8 9, label %for.end
  ]

for.body:                                         ; preds = %land.lhs.true
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp7 = icmp slt i64 %indvars.iv, 14
  br i1 %cmp7, label %land.lhs.true, label %for.end

for.end:                                          ; preds = %land.lhs.true, %land.lhs.true, %land.lhs.true, %for.body
  %idxprom.lcssa = phi i64 [ %indvars.iv, %land.lhs.true ], [ %indvars.iv, %land.lhs.true ], [ %indvars.iv, %land.lhs.true ], [ %indvars.iv.next, %for.body ]
  ret void
}
