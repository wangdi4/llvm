; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -print-after=VPlanDriverHIR -vplan-force-vf=8 < %s 2>&1 | FileCheck %s

; Verify that vectorizer generates a unit stride store for this loop because %s.0 is known positive.

; + DO i1 = 0, -1 * %s.0 + %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967294>
; |   (%p)[i1 + %s.0] = i1 + %s.0;
; + END LOOP

; CHECK: + DO i1 = 0, 8 * %tgu + -1, 8   <DO_LOOP>  <MAX_TC_EST = 536870911> <nounroll>
; CHECK: |   (<8 x i32>*)(%p)[i1 + %s.0] = i1 + %s.0 + <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>;
; CHECK: + END LOOP


define dso_local void @foo(i32 %t, i32 %N, i32* %p) {
entry:
  %cmp = icmp eq i32 %t, 2
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  br label %if.end3

if.else:                                          ; preds = %entry
  %cmp1 = icmp eq i32 %t, 3
  br i1 %cmp1, label %if.then2, label %if.end3

if.then2:                                         ; preds = %if.else
  br label %if.end3

if.end3:                                          ; preds = %if.then2, %if.else, %if.then
  %s.0 = phi i32 [ 3, %if.then ], [ 5, %if.then2 ], [ 1, %if.else ]
  %cmp41 = icmp ult i32 %s.0, %N
  br i1 %cmp41, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %if.end3
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %i.02 = phi i32 [ %s.0, %for.body.lr.ph ], [ %inc, %for.body ]
  %idxprom = zext i32 %i.02 to i64
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %idxprom
  store i32 %i.02, i32* %arrayidx, align 4
  %inc = add i32 %i.02, 1
  %cmp4 = icmp ult i32 %inc, %N
  br i1 %cmp4, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %if.end3
  ret void
}
