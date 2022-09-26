;
; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -disable-output -print-after=hir-vplan-vec  -vplan-force-vf=4 -hir-details -vplan-force-invariant-decomposition < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -disable-output -vplan-force-vf=4 -hir-details -vplan-force-invariant-decomposition < %s 2>&1 | FileCheck %s
;
; LIT test to check consistency of refs used in vector loop top test. The scalar
; refs generated in the multiply were being marked as linear at i1 loop level
; which is incorrect.
;
; Incoming HIR:
;
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   %0 = (%lpp)[i1];
;       |   %1 = (%lp1)[i1];
;       |   %2 = (%lp2)[i1];
;       |   %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;       |
;       |   + DO i2 = 0, (%1 * %2) + -1, 1   <DO_LOOP>
;       |   |   (%0)[i2] = i2;
;       |   + END LOOP
;       |
;       |   @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
;       + END LOOP
;
; CHECK:   Function: foo
;
; CHECK:           |      %.scal = [[TMP1:%.*]]  *  [[TMP2:%.*]];
; CHECK-NEXT:      |      <LVAL-REG> NON-LINEAR i64 %.scal {sb:25}
; CHECK-NEXT:      |      <RVAL-REG> NON-LINEAR i64 [[TMP1]] {sb:9}
; CHECK-NEXT:      |      <RVAL-REG> NON-LINEAR i64 [[TMP2]] {sb:12}
;
; CHECK:           |      %tgu3 = %.scal  /u  4;
;
; CHECK:           |      %vec.tc4 = %tgu3  *  4;
;
; CHECK:           |      + DO i64 i2 = 0, [[LOOP_UB0:%.*]], 4   <DO_LOOP> <auto-vectorized> <nounroll> <novectorize>
;
define void @foo(i64** %lpp, i64* %lp1, i64* %lp2) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.end
  %l1.024 = phi i64 [ 0, %entry ], [ %inc8, %for.end ]
  %arrayidx = getelementptr inbounds i64*, i64** %lpp, i64 %l1.024
  %0 = load i64*, i64** %arrayidx, align 8
  %arrayidx1 = getelementptr inbounds i64, i64* %lp1, i64 %l1.024
  %1 = load i64, i64* %arrayidx1, align 8
  %arrayidx2 = getelementptr inbounds i64, i64* %lp2, i64 %l1.024
  %2 = load i64, i64* %arrayidx2, align 8
  %mul = mul nsw i64 %2, %1
  %cmp422 = icmp sgt i64 %mul, 0
  br i1 %cmp422, label %for.body5.preheader, label %for.end

for.body5.preheader:                              ; preds = %for.body
  br label %for.body5

for.body5:                                        ; preds = %for.body5.preheader, %for.body5
  %l2.023 = phi i64 [ %inc, %for.body5 ], [ 0, %for.body5.preheader ]
  %arrayidx6 = getelementptr inbounds i64, i64* %0, i64 %l2.023
  store i64 %l2.023, i64* %arrayidx6, align 8
  %inc = add nuw nsw i64 %l2.023, 1
  %exitcond.not = icmp eq i64 %inc, %mul
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body5

for.end.loopexit:                                 ; preds = %for.body5
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %for.body
  %inc8 = add nuw nsw i64 %l1.024, 1
  %exitcond25.not = icmp eq i64 %inc8, 100
  br i1 %exitcond25.not, label %for.end9, label %for.body

for.end9:                                         ; preds = %for.end
  ret void
}
