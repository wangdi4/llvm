;
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=4 -vplan-force-uf=2 -hir-details-no-verbose-indent -hir-details -disable-output < %s 2>&1 | FileCheck %s
;
; LIT test to demonstrate issue in setting up loop live outs correctly when the
; vector loop is unrolled and reduction partial sums are used. The issue causes
; HIR verification errors and shows up when we hoist out reduction final operations
; outside outer loops when we have a perfect loop nest. In this case, folding kicks
; in for the add instruction used as the operand in the reduction-final instruction.
; We need to add all symbases in the folded add operation as live out.
;
; Relevant VPlan instructionsL
;    [DA: Div] i64 %vp24564 = add i64 %vp15240 i64 %vp24084
;    [DA: Uni] i64 %vp16334 = reduction-final{u_add} i64 %vp24564
;
; The add operation is folded and the generated code looks like:
;    %sum.016 = @llvm.vector.reduce.add.v4i64(%.vec5 + %.vec7);
;
; For the above case, we need to mark symbase(%.vec5) and symbase(%.vec7)
; as live out.
;
; CHECK:     LiveOut symbases: 3, 23, 27
; CHECK:     DO i64 i1 = 0, 1023, 1   <DO_LOOP>
; CHECK:        DO i64 i2 = 0, 1023, 8   <DO_LOOP> <auto-vectorized> <nounroll> <novectorize>
; CHECK:        END LOOP
; CHECK:     END LOOP
; CHECK:     %sum.016 = @llvm.vector.reduce.add.v4i64(%.vec5 + %.vec7);
; CHECK:     <LVAL-REG> NON-LINEAR i64 %sum.016 {sb:3}
; CHECK:     <RVAL-REG> NON-LINEAR <4 x i64> %.vec5 + %.vec7 {sb:2}
; CHECK:        <BLOB> NON-LINEAR <4 x i64> %.vec5 {sb:23}
; CHECK:        <BLOB> NON-LINEAR <4 x i64> %.vec7 {sb:27}
;
@arr = global [1024 x [1024 x i64]] zeroinitializer, align 16

define i64 @foo() {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc5
  %sum.016 = phi i64 [ 0, %entry ], [ %add, %for.inc5 ]
  %l1.015 = phi i64 [ 0, %entry ], [ %inc6, %for.inc5 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %sum.114 = phi i64 [ %sum.016, %for.cond1.preheader ], [ %add, %for.body3 ]
  %l2.013 = phi i64 [ 0, %for.cond1.preheader ], [ %inc, %for.body3 ]
  %arrayidx4 = getelementptr inbounds [1024 x [1024 x i64]], ptr @arr, i64 0, i64 %l1.015, i64 %l2.013
  %0 = load i64, ptr %arrayidx4, align 8
  %add = add nsw i64 %0, %sum.114
  %inc = add nuw nsw i64 %l2.013, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.inc5, label %for.body3

for.inc5:                                         ; preds = %for.body3
  %inc6 = add nuw nsw i64 %l1.015, 1
  %exitcond17.not = icmp eq i64 %inc6, 1024
  br i1 %exitcond17.not, label %for.end7, label %for.cond1.preheader

for.end7:                                         ; preds = %for.inc5
  ret i64 %add
}
