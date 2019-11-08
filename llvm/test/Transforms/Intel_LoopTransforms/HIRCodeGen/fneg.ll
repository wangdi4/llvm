; RUN: opt < %s -hir-ssa-deconstruction -analyze -hir-framework -hir-cg -print-after=hir-cg -force-hir-cg 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-cg,print" < %s -force-hir-cg 2>&1 | FileCheck %s

; Verify that we form a loop containing fneg isntruction and successfully generate code for it.

; CHECK: + DO i1 = 0, 1, 1   <DO_LOOP>
; CHECK: |   %conv57 = sitofp.i32.double(i1 + 1);
; CHECK: |   %mul58 = %div40  *  %conv57;
; CHECK: |   %neg =  - %mul58;
; CHECK: |   %mul.i284 = %mul58  *  %neg;
; CHECK: + END LOOP

; Verify the CG generates an fneg instruction.

; CHECK: %fneg = fneg double

define void @foo(double %div40) {
entry:
  br label %for.body46

for.body46:                                       ; preds = %for.body46, %entry
  %I.1289 = phi i32 [ 1, %entry ], [ %inc63, %for.body46 ]
  %conv57 = sitofp i32 %I.1289 to double
  %mul58 = fmul double %div40, %conv57
  %neg = fneg double %mul58
  %mul.i284 = fmul double %mul58, %neg
  %inc63 = add nuw nsw i32 %I.1289, 1
  %exitcond = icmp eq i32 %inc63, 3
  br i1 %exitcond, label %exit, label %for.body46

exit:
  %add61.lcssa = phi double [ %mul.i284, %for.body46 ]
  ret void
}
