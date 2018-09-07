; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -print-after=VPlanDriverHIR -vplan-force-vf=8 < %s 2>&1 | FileCheck %s

; Verify that vectorizer generates a unit stride store for this loop because umin blob (-1 * umax) is known positive.

; + DO i1 = 0, umax((-1 + (-1 * %t2)), (-1 + (-1 * %t1))) + %N, 1   <DO_LOOP>
; |   (%p)[i1 + (-1 * umax((-1 + (-1 * %t2)), (-1 + (-1 * %t1)))) + -1] = i1 + (-1 * umax((-1 + (-1 * %t2)), (-1 + (-1 * %t1)))) + -1;
; + END LOOP

; CHECK: + DO i1 = 0, 8 * %tgu + -1, 8   <DO_LOOP> <nounroll>
; CHECK: |   (<8 x i32>*)(%p)[i1 + (-1 * umax((-1 + (-1 * %t2)), (-1 + (-1 * %t1)))) + -1] = i1 + (-1 * umax((-1 + (-1 * %t2)), (-1 + (-1 * %t1)))) + <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7> + -1;
; CHECK: + END LOOP


define dso_local void @foo(i32 %t1, i32 %t2, i32 %N, i32* nocapture %p) {
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
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %idxprom
  store i32 %i.010, i32* %arrayidx, align 4
  %inc = add i32 %i.010, 1
  %cmp1 = icmp ult i32 %inc, %N
  br i1 %cmp1, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

