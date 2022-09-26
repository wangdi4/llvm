; Test to verify that VPlan HIR vectorizer codegen does not bail out for loops
; containing unconditional liveouts and calls to ignored functions.

; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-force-vf=4 -disable-output -print-after=hir-vplan-vec < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s


define float @foo1(float* nocapture %a, float %const, i64* %lt.arg) {
; CHECK:          BEGIN REGION { modified }
; CHECK-NEXT:           + DO i1 = 0, 99, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK-NEXT:           |   %.vec = bitcast.<4 x i64*>.<4 x i8*>(%lt.arg);
; CHECK-NEXT:           |   %.vec2 = (<4 x float>*)(%a)[i1];
; CHECK-NEXT:           |   %.vec3 = %.vec2  +  %const;
; CHECK-NEXT:           |   (<4 x float>*)(%a)[i1] = %.vec3;
; CHECK-NEXT:           |   %.vec4 = bitcast.<4 x i64*>.<4 x i8*>(%lt.arg);
; CHECK-NEXT:           + END LOOP

; CHECK:                %0 = extractelement %.vec2,  3;
; CHECK:                ret %0;
; CHECK-NEXT:     END REGION

entry:
  %lt.var = bitcast i64* %lt.arg to i8*
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %l1.010 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %lt.var) #2
  %arrayidx = getelementptr inbounds float, float* %a, i64 %l1.010
  %0 = load float, float* %arrayidx, align 8
  ;%call = tail call fast float @llvm.sin.f32(float %0)
  %add = fadd fast float %0, %const
  store float %add, float* %arrayidx, align 8
  %inc = add nuw nsw i64 %l1.010, 1
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %lt.var) #2
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret float %0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
