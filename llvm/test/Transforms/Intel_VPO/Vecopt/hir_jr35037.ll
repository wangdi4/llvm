; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s
;
; LIT test for the regression reported in CMPLRLLVM-35037. The changes to stop
; tracking IV updates introduced a regression in HIR vector code generation in
; the non-merged CFG path. This path uses HIR utilities to generate main vector
; loop and skip generating code for loop-related instructions as the generated
; instructions would be redundant. However, the changes avoid skipping over
; loop related add instructions as we need the generated instructions to handle
; unrolled vector loops. This caused issues for cases like the following loop
; related instructions which calculate trip count as we skip generating code for
; the sext used as an operand of the add instruction.
;
;   [DA: Uni, SVA: (F  )] i64 %vp21436 = sext i32 %i1 to i64 (SVAOpBits 0->F )
;   [DA: Uni, SVA: (F  )] i64 %vp21410 = add i64 %vp21436 i64 1 (SVAOpBits 0->F 1->F )
;   [DA: Uni, SVA: (F  )] i64 %vp25046 = vector-trip-count i64 %vp21410, UF = 1 (SVAOpBits 0->F )
;
; The fix skips all loop related instructions outside the top-level VPLoop
; for non merged-CFG path. Merged CFG path CG will generate code for all
; VPInstructions and the fix is a temporary solution until we have this in
; place. The test checks that inner loop is successfully vectorized.
;
; Incoming HIR looks like the following(outer loop IV is i32 type and inner
; loop IV is i64 type).
;
;              + DO i1 = 0, 39, 1   <DO_LOOP>
;              |   %0 = (%lpp)[i1];
;              |   %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;              |
;              |   + DO i2 = 0, i1, 1   <DO_LOOP>  <MAX_TC_EST = 40>  <LEGAL_MAX_TC = 40>
;              |   |   (%0)[i2] = i2;
;              |   + END LOOP
;              |
;              |   @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
;              + END LOOP
;
; CHECK:       + DO i2 = 0, {{.*}}, 4   <DO_LOOP>  <MAX_TC_EST = 10>  <LEGAL_MAX_TC = 10> <auto-vectorized> <nounroll> <novectorize>
; CHECK-NEXT:  |   (<4 x i64>*)([[TMP:%.*]])[i2] = i2 + <i64 0, i64 1, i64 2, i64 3>;
; CHECK-NEXT:  + END LOOP
;
define void @foo(i64** %lpp) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.end
  %indvars.iv = phi i32 [ 1, %entry ], [ %indvars.iv.next, %for.end ]
  %l1.018 = phi i32 [ 0, %entry ], [ %inc6, %for.end ]
  %indvars.iv.ext = sext i32 %indvars.iv to i64
  %l1.018.ext = sext i32 %l1.018 to i64
  %arrayidx = getelementptr inbounds i64*, i64** %lpp, i64 %l1.018.ext
  %0 = load i64*, i64** %arrayidx, align 8
  br label %for.body3

for.body3:                                        ; preds = %for.body, %for.body3
  %l2.017 = phi i64 [ 0, %for.body ], [ %inc, %for.body3 ]
  %arrayidx4 = getelementptr inbounds i64, i64* %0, i64 %l2.017
  store i64 %l2.017, i64* %arrayidx4, align 8
  %inc = add nuw nsw i64 %l2.017, 1
  %exitcond = icmp eq i64 %inc, %indvars.iv.ext
  br i1 %exitcond, label %for.end, label %for.body3

for.end:                                          ; preds = %for.body3
  %inc6 = add nuw nsw i32 %l1.018, 1
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond19.not = icmp eq i32 %inc6, 40
  br i1 %exitcond19.not, label %for.end7, label %for.body

for.end7:                                         ; preds = %for.end
  ret void
}
