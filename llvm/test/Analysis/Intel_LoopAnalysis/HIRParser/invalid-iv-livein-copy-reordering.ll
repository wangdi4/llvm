; RUN: opt -passes="hir-ssa-deconstruction,print<hir>" -hir-framework-debug=parser -disable-output < %s 2>&1 | FileCheck %s

; Verify that the definition of %iv is after %i23 at the end of i2 loop.
; Both of them are livein copies for the corresponding header phis. The livein
; copy of %i23 uses %iv in rval. %iv was incorrectly being reordered before %i23
; as %i23 was considered a IV livein copy and we have logic to keep those copies
; at the end. The logic was missing the check that %iv is itself an IV and
; doesn't need to be reordered.

; CHECK: + DO i1 = 0, 20, 1   <DO_LOOP>
; CHECK: |   %i25 = (%s1)[0][-1 * i1 + 21];
; CHECK: |   %i26 = (%t3)[0][-1 * i1 + 22];
; CHECK: |   (%t3)[0][-1 * i1 + 22] = (%i26 * %i25 * %i23 * %iv);
; CHECK: |   %i31 = (%g)[0][-1 * i1 + 22][-1 * i1 + 21];
; CHECK: |
; CHECK: |   + DO i2 = 0, i1 + 11, 1   <DO_LOOP>  <MAX_TC_EST = 32>  <LEGAL_MAX_TC = 32>
; CHECK: |   |   %i38 = (%le0)[0][-1 * i2 + 34][-1 * i1 + 23];
; CHECK: |   |   %i39 = (%sd)[0][-1 * i2 + 34];
; CHECK: |   |   (%sd)[0][-1 * i2 + 34] = -1 * %i38 + %i39;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %i23 = -34 * i1 + -34 * %i31 + -34 * %nh.promoted + ((11 + (-1 * %nh.promoted) + %iv) * (-31 + %i31 + %iv)) + 1054;
; CHECK: |   %iv = i1 + %nh.promoted + 1;
; CHECK: + END LOOP


define void @foo(i32 %i21, i32 %nh.promoted, i32 %nv.promoted, ptr %of8, ptr %s1, ptr %t3, ptr %sd, ptr %g, ptr %le0, ptr %arrayidx) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.cond.loopexit, %entry
  %i23 = phi i32 [ %i21, %entry ], [ %mul45.lcssa, %for.cond.loopexit ]
  %indvars.iv272 = phi i64 [ 22, %entry ], [ %indvars.iv.next273, %for.cond.loopexit ]
  %iv = phi i32 [ %nh.promoted, %entry ], [ %inc, %for.cond.loopexit ]
  %inc18267269 = phi i32 [ %nv.promoted, %entry ], [ %inc18, %for.cond.loopexit ]
  %indvars.iv.next273 = add nsw i64 %indvars.iv272, -1
  %inc = add i32 %iv, 1
  %arrayidx11 = getelementptr inbounds [100 x i32], ptr %s1, i64 0, i64 %indvars.iv.next273
  %i25 = load i32, ptr %arrayidx11, align 4
  %arrayidx14 = getelementptr inbounds [100 x i32], ptr %t3, i64 0, i64 %indvars.iv272
  %i26 = load i32, ptr %arrayidx14, align 4
  %mul = mul i32 %iv, %i23
  %mul12 = mul i32 %mul, %i25
  %mul15 = mul i32 %mul12, %i26
  store i32 %mul15, ptr %arrayidx14, align 4
  %inc18 = add i32 %inc18267269, 1
  br label %for.body33.lr.ph

for.body33.lr.ph:                                 ; preds = %if.then, %for.body
  %arrayidx42 = getelementptr inbounds [100 x [100 x i32]], ptr %g, i64 0, i64 %indvars.iv272, i64 %indvars.iv.next273
  %i31 = load i32, ptr %arrayidx42, align 4
  %i32 = add i32 %iv, %i31
  %sub44 = sub i32 31, %i32
  %inc18.neg = xor i32 %inc18267269, -1
  %i33 = trunc i64 %indvars.iv272 to i32
  %sub46.neg = sub i32 %inc, %i33
  %add47.neg = add i32 %sub46.neg, %inc18.neg
  %i34 = add nuw nsw i64 %indvars.iv272, 1
  br label %for.body33

for.body33:                                       ; preds = %for.body33, %for.body33.lr.ph
  %indvars.iv = phi i64 [ 34, %for.body33.lr.ph ], [ %indvars.iv.next, %for.body33 ]
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  %arrayidx54 = getelementptr inbounds [100 x [100 x i32]], ptr %le0, i64 0, i64 %indvars.iv, i64 %i34
  %i38 = load i32, ptr %arrayidx54, align 4
  %arrayidx56 = getelementptr inbounds [100 x i32], ptr %sd, i64 0, i64 %indvars.iv
  %i39 = load i32, ptr %arrayidx56, align 4
  %sub57 = sub i32 %i39, %i38
  store i32 %sub57, ptr %arrayidx56, align 4
  %i37 = trunc i64 %indvars.iv to i32
  %mul45 = mul i32 %sub44, %i37
  %cmp32 = icmp ugt i64 %indvars.iv.next, %indvars.iv272
  br i1 %cmp32, label %for.body33, label %for.cond.loopexit

for.cond.loopexit:                                ; preds = %for.body33
  %indvars.iv.next.lcssa = phi i64 [ %indvars.iv.next, %for.body33 ]
  %mul45.lcssa = phi i32 [ %mul45, %for.body33 ]
  %cmp = icmp ugt i64 %indvars.iv.next273, 1
  br i1 %cmp, label %for.body, label %for.end60

for.end60:
  ret void
}
