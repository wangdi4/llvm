; Test to verify that VPlan HIR vectorizer codegen handles loops containing
; unconditional liveouts and call instructions.

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -disable-output -print-after=hir-vplan-vec < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s

define float @foo1(float* nocapture %a) {
; CHECK-LABEL:   BEGIN REGION { modified }
; CHECK-NEXT:          + DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:          |   %.vec = (<4 x float>*)(%a)[i1];
; CHECK-NEXT:          |   %llvm.sin.v4f32 = @llvm.sin.v4f32(%.vec);
; CHECK-NEXT:          |   %.vec1 = %.vec  +  %llvm.sin.v4f32;
; CHECK-NEXT:          |   (<4 x float>*)(%a)[i1] = %.vec1;
; CHECK-NEXT:          + END LOOP
; CHECK:               %0 = extractelement %.vec,  3;
; CHECK:         END REGION
;
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
; CHECK-LABEL:   BEGIN REGION { modified }
; CHECK-NEXT:          + DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:          |   %.vec = (<4 x float>*)(%a)[i1];
; CHECK-NEXT:          |   %llvm.sin.v4f32 = @llvm.sin.v4f32(%.vec);
; CHECK-NEXT:          |   %.vec1 = %.vec  +  %llvm.sin.v4f32;
; CHECK-NEXT:          |   (<4 x float>*)(%a)[i1] = %.vec1;
; CHECK-NEXT:          + END LOOP
; CHECK:               %add = extractelement %.vec1,  3;
; CHECK:         END REGION
;
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
