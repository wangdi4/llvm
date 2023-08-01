; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Check parsing output for the loop verifying that the subscripts are successfully parsed in terms of outer loop IV(i1).

; CHECK: + DO i1 = 0, %dual.0.i85 + -2, 1   <DO_LOOP>
; CHECK: |   %0 = (%twp.addr.0.i89)[2 * i1];
; CHECK: |   %1 = (%twp.addr.0.i89)[2 * i1 + 1];
; CHECK: |
; CHECK: |   + DO i2 = 0, smax(1, %tc) + -1, 1   <DO_LOOP>
; CHECK: |   |   %2 = (%dp)[2 * i1 + 2 * %add79 * i2 + 2 * %dual.0.i85 + 2];
; CHECK: |   |   %3 = (%dp)[2 * i1 + 2 * %add79 * i2 + 2 * %dual.0.i85 + 3];
; CHECK: |   |   %mul52.i.us = %0  *  %2;
; CHECK: |   |   %mul53.i.us = %1  *  %3;
; CHECK: |   |   %sub54.i.us = %mul52.i.us  -  %mul53.i.us;
; CHECK: |   |   %mul56.i.us = %0  *  %3;
; CHECK: |   |   %mul57.i.us = %1  *  %2;
; CHECK: |   |   %add58.i.us = %mul57.i.us  +  %mul56.i.us;
; CHECK: |   |   %4 = (%dp)[2 * i1 + 2 * %add79 * i2 + 2];
; CHECK: |   |   %sub61.i.us = %4  -  %sub54.i.us;
; CHECK: |   |   (%dp)[2 * i1 + 2 * %add79 * i2 + 2 * %dual.0.i85 + 2] = %sub61.i.us;
; CHECK: |   |   %5 = (%dp)[2 * i1 + 2 * %add79 * i2 + 3];
; CHECK: |   |   %sub67.i.us = %5  -  %add58.i.us;
; CHECK: |   |   (%dp)[2 * i1 + 2 * %add79 * i2 + 2 * %dual.0.i85 + 3] = %sub67.i.us;
; CHECK: |   |   %6 = (%dp)[2 * i1 + 2 * %add79 * i2 + 2];
; CHECK: |   |   %add73.i.us = %sub54.i.us  +  %6;
; CHECK: |   |   (%dp)[2 * i1 + 2 * %add79 * i2 + 2] = %add73.i.us;
; CHECK: |   |   %7 = (%dp)[2 * i1 + 2 * %add79 * i2 + 3];
; CHECK: |   |   %add77.i.us = %add58.i.us  +  %7;
; CHECK: |   |   (%dp)[2 * i1 + 2 * %add79 * i2 + 3] = %add77.i.us;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


define void @foo(ptr %twp.addr.0.i89, ptr %dp, i32 %dual.0.i85, i32 %add79, i64 %tc) {
entry:
  br label %for.body34.i.us

for.body34.i.us:                                  ; preds = %for.cond36.i.for.inc82.i_crit_edge.us, %entry
  %twp.addr.1.i82.us = phi ptr [ %incdec.ptr35.i.us, %for.cond36.i.for.inc82.i_crit_edge.us ], [ %twp.addr.0.i89, %entry ]
  %a.0.i80.us = phi i32 [ %inc.i.us, %for.cond36.i.for.inc82.i_crit_edge.us ], [ 1, %entry ]
  %incdec.ptr.i.us = getelementptr inbounds double, ptr %twp.addr.1.i82.us, i64 1
  %0 = load double, ptr %twp.addr.1.i82.us, align 8
  %1 = load double, ptr %incdec.ptr.i.us, align 8
  br label %for.body38.i.us

for.body38.i.us:                                  ; preds = %for.body34.i.us, %for.body38.i.us
  %indvars.iv110 = phi i64 [ 0, %for.body34.i.us ], [ %indvars.iv.next111, %for.body38.i.us ]
  %b.1.i78.us = phi i32 [ 0, %for.body34.i.us ], [ %add80.i.us, %for.body38.i.us ]
  %add40.i.us = add nsw i32 %b.1.i78.us, %a.0.i80.us
  %mul41.i.us = shl nsw i32 %add40.i.us, 1
  %add44.i.us = add nsw i32 %add40.i.us, %dual.0.i85
  %mul45.i.us = shl nsw i32 %add44.i.us, 1
  %idxprom46.i.us = sext i32 %mul45.i.us to i64
  %arrayidx47.i.us = getelementptr inbounds double, ptr %dp, i64 %idxprom46.i.us
  %2 = load double, ptr %arrayidx47.i.us, align 8
  %add48.i.us = or i32 %mul45.i.us, 1
  %idxprom49.i.us = sext i32 %add48.i.us to i64
  %arrayidx50.i.us = getelementptr inbounds double, ptr %dp, i64 %idxprom49.i.us
  %3 = load double, ptr %arrayidx50.i.us, align 8
  %mul52.i.us = fmul double %0, %2
  %mul53.i.us = fmul double %1, %3
  %sub54.i.us = fsub double %mul52.i.us, %mul53.i.us
  %mul56.i.us = fmul double %0, %3
  %mul57.i.us = fmul double %1, %2
  %add58.i.us = fadd double %mul57.i.us, %mul56.i.us
  %idxprom59.i.us = sext i32 %mul41.i.us to i64
  %arrayidx60.i.us = getelementptr inbounds double, ptr %dp, i64 %idxprom59.i.us
  %4 = load double, ptr %arrayidx60.i.us, align 8
  %sub61.i.us = fsub double %4, %sub54.i.us
  store double %sub61.i.us, ptr %arrayidx47.i.us, align 8
  %add64.i.us = or i32 %mul41.i.us, 1
  %idxprom65.i.us = sext i32 %add64.i.us to i64
  %arrayidx66.i.us = getelementptr inbounds double, ptr %dp, i64 %idxprom65.i.us
  %5 = load double, ptr %arrayidx66.i.us, align 8
  %sub67.i.us = fsub double %5, %add58.i.us
  store double %sub67.i.us, ptr %arrayidx50.i.us, align 8
  %6 = load double, ptr %arrayidx60.i.us, align 8
  %add73.i.us = fadd double %sub54.i.us, %6
  store double %add73.i.us, ptr %arrayidx60.i.us, align 8
  %7 = load double, ptr %arrayidx66.i.us, align 8
  %add77.i.us = fadd double %add58.i.us, %7
  store double %add77.i.us, ptr %arrayidx66.i.us, align 8
  %indvars.iv.next111 = add nsw i64 %indvars.iv110, 1
  %add80.i.us = add nsw i32 %b.1.i78.us, %add79
  %cmp37.i.us = icmp slt i64 %indvars.iv.next111, %tc
  br i1 %cmp37.i.us, label %for.body38.i.us, label %for.cond36.i.for.inc82.i_crit_edge.us

for.cond36.i.for.inc82.i_crit_edge.us:            ; preds = %for.body38.i.us
  %incdec.ptr35.i.us = getelementptr inbounds double, ptr %twp.addr.1.i82.us, i64 2
  %inc.i.us = add nuw nsw i32 %a.0.i80.us, 1
  %exitcond = icmp eq i32 %inc.i.us, %dual.0.i85
  br i1 %exitcond, label %for.end83.i.loopexit, label %for.body34.i.us

for.end83.i.loopexit:
  ret void
}
