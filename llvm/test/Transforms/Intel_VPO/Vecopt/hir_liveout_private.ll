; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -enable-vp-value-codegen-hir=0 -disable-output -print-after=VPlanDriverHIR < %s 2>&1 | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -enable-vp-value-codegen-hir=1 -disable-output -print-after=VPlanDriverHIR < %s 2>&1 | FileCheck %s
;
; LIT test to check code generated for liveout private.
; VPValue based code generation is not implemented for the same currently as
; liveout private support needs to be made explicit in VPlan. Until then we
; force mixed code generation for such cases. The test is checking that the
; vector loop generated is same as in mixed CG for this case.
;
define i64 @foo(i64* nocapture %larr) {
; CHECK-LABEL:  *** IR Dump After VPlan Vectorization Driver HIR ***
; CHECK:                    BEGIN REGION { modified }
; CHECK-NEXT:                     + DO i1 = 0, 99, 4   <DO_LOOP> <novectorize>
; CHECK-NEXT:                     |   [[DOTVEC0:%.*]] = (<4 x i64>*)([[LARR0:%.*]])[i1]
; CHECK-NEXT:                     |   (<4 x i64>*)([[LARR0]])[i1] = i1 + <i64 0, i64 1, i64 2, i64 3> + [[DOTVEC0]]
; CHECK-NEXT:                     + END LOOP
; CHECK:                          [[TMP0:%.*]] = extractelement [[DOTVEC0]],  3
; CHECK-NEXT:               END REGION
;
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %l1.010 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds i64, i64* %larr, i64 %l1.010
  %0 = load i64, i64* %arrayidx, align 8
  %add = add nsw i64 %0, %l1.010
  store i64 %add, i64* %arrayidx, align 8
  %inc = add nuw nsw i64 %l1.010, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i64 %0
}
