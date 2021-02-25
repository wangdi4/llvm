; Test to verify that VPlan HIR vectorizer codegen bails out for loops containing
; unconditional liveouts and call instructions. We do not support call vectorization
; feature in mixed CG mode which is currently used when candidate loop contains an
; unconditional liveout.

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -disable-output -print-after=VPlanDriverHIR -debug-only=VPOCGHIR -debug-only=VPOCGHIR-bailout < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,vplan-driver-hir,print<hir>" -vplan-force-vf=4 -disable-output -debug-only=VPOCGHIR -debug-only=VPOCGHIR-bailout < %s 2>&1 | FileCheck %s


define float @foo1(float* nocapture %a) {
; CHECK: VPLAN_OPTREPORT: VPValCG liveout induction/private not handled - forcing mixed CG
; CHECK: VPLAN_OPTREPORT: Loop not handled - call vectorization not supported in mixed CG mode

; CHECK-LABEL:   BEGIN REGION { }
; CHECK-NEXT:          + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK-NEXT:          |   %0 = (%a)[i1];
; CHECK-NEXT:          |   %call = @llvm.sin.f32(%0);
; CHECK-NEXT:          |   %add = %0  +  %call;
; CHECK-NEXT:          |   (%a)[i1] = %add;
; CHECK-NEXT:          + END LOOP
; CHECK-NEXT:    END REGION
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %l1.010 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds float, float* %a, i64 %l1.010
  %0 = load float, float* %arrayidx, align 8
  %call = tail call fast float @llvm.sin.f32(float %0)
  %add = fadd fast float %0, %call
  store float %add, float* %arrayidx, align 8
  %inc = add nuw nsw i64 %l1.010, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret float %0
}

define float @foo2(float* nocapture %a) {
; CHECK: VPLAN_OPTREPORT: VPValCG liveout induction/private not handled - forcing mixed CG
; CHECK: VPLAN_OPTREPORT: Loop not handled - call vectorization not supported in mixed CG mode

; CHECK-LABEL:   BEGIN REGION { }
; CHECK-NEXT:          + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK-NEXT:          |   %0 = (%a)[i1];
; CHECK-NEXT:          |   %call = @llvm.sin.f32(%0);
; CHECK-NEXT:          |   %add = %0  +  %call;
; CHECK-NEXT:          |   (%a)[i1] = %add;
; CHECK-NEXT:          + END LOOP
; CHECK-NEXT:    END REGION
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %l1.010 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds float, float* %a, i64 %l1.010
  %0 = load float, float* %arrayidx, align 8
  %call = tail call fast float @llvm.sin.f32(float %0)
  %add = fadd fast float %0, %call
  store float %add, float* %arrayidx, align 8
  %inc = add nuw nsw i64 %l1.010, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret float %add
}

declare float @llvm.sin.f32(float %Val) nounwind readnone
