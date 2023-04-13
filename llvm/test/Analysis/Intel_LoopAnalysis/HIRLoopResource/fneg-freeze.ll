; RUN: opt -passes="hir-ssa-deconstruction,print<hir-loop-resource>" < %s -disable-output 2>&1 | FileCheck %s

; Verify that we can successfully compute loop resource for loop containing
; fneg and freeze instruction. 
; Note that fneg has a cost of 1 and freeze is considered to have zero cost.

; + DO i1 = 0, 1, 1   <DO_LOOP>
; |   %conv57 = sitofp.i32.double(i1 + 1);
; |   %mul58 = %div40  *  %conv57;
; |   %fr = freeze(%mul58);
; |   %neg =  - %fr;
; |   %mul.i284 = %mul58  *  %neg;
; + END LOOP


; CHECK: + DO i1 = 0, 1, 1   <DO_LOOP>
; CHECK:    Integer Operations: 1
; CHECK:    Integer Operations Cost: 1
; CHECK:    Floating Point Operations: 5
; CHECK:    Floating Point Operations Cost: 4
; CHECK:    Total Cost: 5
; CHECK:    Floating Point Bound
; CHECK: + END LOOP


define void @foo(double %div40) {
entry:
  br label %for.body46

for.body46:                                       ; preds = %for.body46, %entry
  %I.1289 = phi i32 [ 1, %entry ], [ %inc63, %for.body46 ]
  %conv57 = sitofp i32 %I.1289 to double
  %mul58 = fmul double %div40, %conv57
  %fr = freeze double %mul58
  %neg = fneg double %fr
  %mul.i284 = fmul double %mul58, %neg
  %inc63 = add nuw nsw i32 %I.1289, 1
  %exitcond = icmp eq i32 %inc63, 3
  br i1 %exitcond, label %exit, label %for.body46

exit:
  %add61.lcssa = phi double [ %mul.i284, %for.body46 ]
  ret void
}
