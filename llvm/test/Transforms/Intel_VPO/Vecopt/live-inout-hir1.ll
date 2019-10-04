; REQUIRES: asserts
;RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-build -debug -vplan-plain-dump=true -vplan-dump-liveness=1 %s  2>&1 | FileCheck %s

; verify live-in and live-out analysis
; CHECK:Live-in and Live-out info:
; CHECK-NEXT:External defs:
; CHECK-DAG: i64 %n
; CHECK-DAG: i64 %4
; CHECK-DAG: i64* %ub
; CHECK-DAG: i64* %k
; CHECK-DAG: [101 x float]* %a
; CHECK-DAG: [101 x float]* %B
; CHECK-NEXT:Used externally:
; CHECK-DAG: i64 {{%vp.*}}
; CHECK-DAG: i64 {{%vp.*}}
; CHECK-DAG: i64 {{%vp.*}}
; CHECK-DAG: i64 {{%vp.*}} liveout in the loop: BB3
; CHECK-DAG: i64* %k livein in the loops:  BB3
; CHECK-DAG: i64* %k livein in the loops:  BB3
; CHECK-DAG: i64* %ub livein in the loops:  BB3
; CHECK-DAG: i64 %n livein in the loops:  BB3
; CHECK-DAG: i64 {{%vp.*}} livein in the loops:  BB5
; CHECK-DAG: i1 {{%vp.*}} liveout in the loop: BB5
; CHECK-DAG: i64 {{%vp.*}} livein in the loops:  BB5
; CHECK-DAG: i64 {{%vp.*}} livein in the loops:  BB5
; CHECK-DAG: i64 {{%vp.*}} livein in the loops:  BB5
; CHECK-DAG: [101 x float]* %a livein in the loops:  BB5 BB3
; CHECK-DAG: i64 {{%vp.*}} liveout in the loop: BB3
; CHECK-DAG: i64 %n livein in the loops:  BB3
; CHECK-DAG: [101 x float]* %B livein in the loops:  BB3
; CHECK-DAG: [101 x float]* %B livein in the loops:  BB3
; CHECK-DAG: i64 {{%vp.*}} liveout in the loop: BB3
; CHECK-DAG: i64 %4 livein in the loops:  BB3
; CHECK-DAG: i64* %k livein in the loops:  BB3
; CHECK-NEXT:Live-in and Live-out info end
;
; Function Attrs: nounwind uwtable
define dso_local i64 @_Z3foolPlS_PA101_fS1_(i64 %n, i64* nocapture readnone %lb, i64* nocapture readonly %ub, [101 x float]* nocapture %a, [101 x float]* nocapture %B) local_unnamed_addr #0 {
entry:
  %ret = alloca i64, align 8
  %.omp.iv = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %k = alloca i64, align 8
  %0 = bitcast i64* %ret to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %0) #2
  store i64 0, i64* %ret, align 8, !tbaa !2
  %1 = bitcast i64* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %1) #2
  %cmp = icmp sgt i64 %n, 0
  br i1 %cmp, label %omp.precond.then, label %entry.omp.precond.end_crit_edge

entry.omp.precond.end_crit_edge:                  ; preds = %entry
  %.pre41 = bitcast i64* %.omp.ub to i8*
  br label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %sub2 = add nsw i64 %n, -1
  %2 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %2) #2
  store i64 %sub2, i64* %.omp.ub, align 8, !tbaa !2
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.LASTPRIVATE"(i64* %ret), "QUAL.OMP.NORMALIZED.IV"(i64* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i64* %.omp.ub), "QUAL.OMP.PRIVATE"(i64* %k) ]
  store i64 0, i64* %.omp.iv, align 8, !tbaa !2
  %4 = load i64, i64* %.omp.ub, align 8, !tbaa !2
  %cmp437 = icmp slt i64 %4, 0
  br i1 %cmp437, label %omp.loop.exit, label %omp.inner.for.body.lr.ph

omp.inner.for.body.lr.ph:                         ; preds = %omp.precond.then
  %5 = bitcast i64* %k to i8*
  %add13 = add nuw nsw i64 %n, 2
  br label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %omp.inner.for.body.lr.ph, %for.end
  %storemerge38 = phi i64 [ 0, %omp.inner.for.body.lr.ph ], [ %add19, %for.end ]
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %5) #2
  store i64 0, i64* %k, align 8, !tbaa !2
  %arrayidx = getelementptr inbounds i64, i64* %ub, i64 %storemerge38
  %6 = load i64, i64* %arrayidx, align 8, !tbaa !2
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %j.036 = phi i64 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %add7 = add nsw i64 %6, %j.036
  %cmp8 = icmp sgt i64 %add7, 0
  %conv = zext i1 %cmp8 to i64
  %add9 = add nuw nsw i64 %storemerge38, %conv
  %conv10 = sitofp i64 %add9 to float
  %arrayidx12 = getelementptr inbounds [101 x float], [101 x float]* %a, i64 %storemerge38, i64 %j.036
  store float %conv10, float* %arrayidx12, align 4, !tbaa !6
  %inc = add nuw nsw i64 %j.036, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %cmp8.lcssa = phi i1 [ %cmp8, %for.body ]
  %conv.le = zext i1 %cmp8.lcssa to i64
  %add14 = add nsw i64 %add13, %conv.le
  %conv15 = sitofp i64 %add14 to float
  %arrayidx17 = getelementptr inbounds [101 x float], [101 x float]* %B, i64 %storemerge38, i64 %storemerge38
  %7 = load float, float* %arrayidx17, align 4, !tbaa !6
  %add18 = fadd float %7, %conv15
  store float %add18, float* %arrayidx17, align 4, !tbaa !6
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %5) #2
  %add19 = add nuw nsw i64 %storemerge38, 1
  %cmp4 = icmp slt i64 %storemerge38, %4
  br i1 %cmp4, label %for.body.lr.ph, label %omp.inner.for.cond.omp.loop.exit_crit_edge

omp.inner.for.cond.omp.loop.exit_crit_edge:       ; preds = %for.end
  %conv.le.lcssa = phi i64 [ %conv.le, %for.end ]
  %add19.lcssa = phi i64 [ %add19, %for.end ]
  %storemerge38.lcssa = phi i64 [ %storemerge38, %for.end ]
  store i64 %conv.le.lcssa, i64* %ret, align 8, !tbaa !2
  store i64 %add19.lcssa, i64* %.omp.iv, align 8, !tbaa !2
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.cond.omp.loop.exit_crit_edge, %omp.precond.then
  %i.0.lcssa = phi i64 [ %storemerge38.lcssa, %omp.inner.for.cond.omp.loop.exit_crit_edge ], [ undef, %omp.precond.then ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SIMD"() ]
  %.pre40 = load i64, i64* %ret, align 8, !tbaa !2
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %entry.omp.precond.end_crit_edge, %omp.loop.exit
  %.pre-phi = phi i8* [ %.pre41, %entry.omp.precond.end_crit_edge ], [ %2, %omp.loop.exit ]
  %8 = phi i64 [ 0, %entry.omp.precond.end_crit_edge ], [ %.pre40, %omp.loop.exit ]
  %i.1 = phi i64 [ undef, %entry.omp.precond.end_crit_edge ], [ %i.0.lcssa, %omp.loop.exit ]
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %.pre-phi) #2
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %1) #2
  %add20 = add nsw i64 %8, %i.1
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %0) #2
  ret i64 %add20
}
; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang ac851cfbcd4a1eacb0869ed900c16428bfa067b4) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm a10a0fd7aff59563c566e2548eb5a0d00bcde577)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"long", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !8, i64 0}
!7 = !{!"array@_ZTSA101_f", !8, i64 0}
!8 = !{!"float", !4, i64 0}


