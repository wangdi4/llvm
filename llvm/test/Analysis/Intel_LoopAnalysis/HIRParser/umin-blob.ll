; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Verify that the livein umin blob umin(%t1, %t2) is reverse engineered into %0.

; CHECK: + DO i1 = 0, -1 * %0 + %N + -1, 1   <DO_LOOP>
; CHECK: |   (%p)[i1 + %0] = i1 + %0;
; CHECK: + END LOOP


define dso_local void @foo(i32 %t1, i32 %t2, i32 %N, ptr nocapture %p) {
entry:
  %cmp = icmp ult i32 %t1, %t2
  %0 = select i1 %cmp, i32 %t1, i32 %t2
  %cmp19 = icmp ult i32 %0, %N
  br i1 %cmp19, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.010 = phi i32 [ %inc, %for.body ], [ %0, %for.body.preheader ]
  %idxprom = zext i32 %i.010 to i64
  %arrayidx = getelementptr inbounds i32, ptr %p, i64 %idxprom
  store i32 %i.010, ptr %arrayidx, align 4
  %inc = add i32 %i.010, 1
  %cmp1 = icmp ult i32 %inc, %N
  br i1 %cmp1, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

