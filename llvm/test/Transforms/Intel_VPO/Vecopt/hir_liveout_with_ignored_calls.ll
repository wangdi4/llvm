    ; Test to verify that VPlan HIR vectorizer codegen does not bail out for loops
; containing unconditional liveouts and calls to ignored functions.

; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s


define float @foo1(ptr nocapture %a, float %const, ptr %lt.arg) {
; CHECK:          BEGIN REGION { modified }
; CHECK-NEXT:           + DO i1 = 0, 99, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK-NEXT:           |   [[VEC:%.*]] = (<4 x float>*)(%a)[i1];
; CHECK-NEXT:           |   [[VEC1:%.*]] = [[VEC]] +  %const;
; CHECK-NEXT:           |   (<4 x float>*)(%a)[i1] = [[VEC1]];
; CHECK-NEXT:           + END LOOP

; CHECK:                %0 = extractelement [[VEC]],  3;
; CHECK:                ret %0;
; CHECK-NEXT:     END REGION

entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %l1.010 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %lt.arg) #2
  %arrayidx = getelementptr inbounds float, ptr %a, i64 %l1.010
  %0 = load float, ptr %arrayidx, align 8
  ;%call = tail call fast float @llvm.sin.f32(float %0)
  %add = fadd fast float %0, %const
  store float %add, ptr %arrayidx, align 8
  %inc = add nuw nsw i64 %l1.010, 1
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %lt.arg) #2
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret float %0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
