; REQUIRES: asserts
; RUN: opt %s -VPlanDriver -vplan-force-build -loopopt=0 -debug -vplan-plain-dump=true -vplan-dump-liveness=1 -disable-vplan-codegen 2>&1 | FileCheck %s 

; verify live-in and live-out analysis

; CHECK:Live-in and Live-out info:
; CHECK-NEXT:External defs:
; CHECK-DAG: i64 %ret.promoted35
; CHECK-DAG: i64 %n
; CHECK-DAG: i64* %ub
; CHECK-DAG: float %conv11
; CHECK-DAG: i64* %k
; CHECK-DAG: i8* %4
; CHECK-DAG: [101 x float]* %B
; CHECK-DAG: i64 %3
; CHECK-NEXT:Used externally:
; CHECK-DAG: i64 {{%vp.*}}
; CHECK-DAG: i64 {{%vp.*}}
; CHECK-DAG: i64 {{%vp.*}}
; CHECK-DAG: i64 %ret.promoted35 livein in the loops:  BB2
; CHECK-DAG: i64 {{%vp.*}} liveout in the loop: BB2
; CHECK-DAG: i8* %4 livein in the loops:  BB2
; CHECK-DAG: i64* %k livein in the loops:  BB2
; CHECK-DAG: i64 {{%vp.*}} livein in the loops:  BB9
; CHECK-DAG: i64* %ub livein in the loops:  BB9 BB2
; CHECK-DAG: i64 {{%vp.*}} liveout in the loop: BB9
; CHECK-DAG: i64 %n livein in the loops:  BB9 BB2
; CHECK-DAG: i64 {{%vp.*}} liveout in the loop: BB2
; CHECK-DAG: [101 x float]* %B livein in the loops:  BB2
; CHECK-DAG: float %conv11 livein in the loops:  BB2
; CHECK-DAG: i8* %4 livein in the loops:  BB2
; CHECK-DAG: i64 {{%vp.*}} liveout in the loop: BB2
; CHECK-DAG: i64 %3 livein in the loops:  BB2
; CHECK-NEXT:Live-in and Live-out info end

; Function Attrs: nounwind uwtable
define dso_local i64 @_Z3foolPlS_PA101_fS1_(i64 %n, i64* nocapture readnone %lb, i64* nocapture readonly %ub, [101 x float]* nocapture readnone %a, [101 x float]* nocapture %B) local_unnamed_addr #0 {
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
  %.pre34 = bitcast i64* %.omp.ub to i8*
  br label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %sub2 = add nsw i64 %n, -1
  %2 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %2) #2
  store i64 %sub2, i64* %.omp.ub, align 8, !tbaa !2
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.precond.then
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.REDUCTION.ADD"(i64* %ret), "QUAL.OMP.NORMALIZED.IV"(i64* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i64* %.omp.ub), "QUAL.OMP.PRIVATE"(i64* %k) ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %DIR.OMP.SIMD.1
  store i64 0, i64* %.omp.iv, align 8, !tbaa !2
  %3 = load i64, i64* %.omp.ub, align 8, !tbaa !2
  %cmp431 = icmp slt i64 %3, 0
  br i1 %cmp431, label %omp.loop.exit, label %omp.inner.for.body.lr.ph

omp.inner.for.body.lr.ph:                         ; preds = %DIR.QUAL.LIST.END.2
  %4 = bitcast i64* %k to i8*
  %add10 = add nuw nsw i64 %n, 2
  %conv11 = sitofp i64 %add10 to float
  %ret.promoted35 = load i64, i64* %ret, align 8, !tbaa !2
  br label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %omp.inner.for.body.lr.ph, %for.end
  %add9.lcssa36 = phi i64 [ %ret.promoted35, %omp.inner.for.body.lr.ph ], [ %add9.lcssa, %for.end ]
  %storemerge32 = phi i64 [ 0, %omp.inner.for.body.lr.ph ], [ %add15, %for.end ]
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %4) #2
  store i64 0, i64* %k, align 8, !tbaa !2
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %add930 = phi i64 [ %add9.lcssa36, %for.body.lr.ph ], [ %add9, %for.body ]
  %j.029 = phi i64 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds i64, i64* %ub, i64 %j.029
  %5 = load i64, i64* %arrayidx, align 8, !tbaa !2
  %add7 = add nsw i64 %5, %j.029
  %cmp8 = icmp sgt i64 %add7, 0
  %conv = zext i1 %cmp8 to i64
  %add9 = add nsw i64 %add930, %conv
  %inc = add nuw nsw i64 %j.029, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %add9.lcssa = phi i64 [ %add9, %for.body ]
  %arrayidx13 = getelementptr inbounds [101 x float], [101 x float]* %B, i64 %storemerge32, i64 %storemerge32
  %6 = load float, float* %arrayidx13, align 4, !tbaa !6
  %add14 = fadd float %6, %conv11
  store float %add14, float* %arrayidx13, align 4, !tbaa !6
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %4) #2
  %add15 = add nuw nsw i64 %storemerge32, 1
  %cmp4 = icmp slt i64 %storemerge32, %3
  br i1 %cmp4, label %for.body.lr.ph, label %omp.inner.for.cond.omp.loop.exit_crit_edge

omp.inner.for.cond.omp.loop.exit_crit_edge:       ; preds = %for.end
  %add15.lcssa = phi i64 [ %add15, %for.end ]
  %storemerge32.lcssa = phi i64 [ %storemerge32, %for.end ]
  %add9.lcssa.lcssa = phi i64 [ %add9.lcssa, %for.end ]
  store i64 %add9.lcssa.lcssa, i64* %ret, align 8, !tbaa !2
  store i64 %add15.lcssa, i64* %.omp.iv, align 8, !tbaa !2
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.cond.omp.loop.exit_crit_edge, %DIR.QUAL.LIST.END.2
  %i.0.lcssa = phi i64 [ %storemerge32.lcssa, %omp.inner.for.cond.omp.loop.exit_crit_edge ], [ undef, %DIR.QUAL.LIST.END.2 ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %omp.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.4

DIR.QUAL.LIST.END.4:                              ; preds = %DIR.OMP.END.SIMD.3
  %.pre = load i64, i64* %ret, align 8, !tbaa !2
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %entry.omp.precond.end_crit_edge, %DIR.QUAL.LIST.END.4
  %.pre-phi = phi i8* [ %.pre34, %entry.omp.precond.end_crit_edge ], [ %2, %DIR.QUAL.LIST.END.4 ]
  %7 = phi i64 [ 0, %entry.omp.precond.end_crit_edge ], [ %.pre, %DIR.QUAL.LIST.END.4 ]
  %i.1 = phi i64 [ undef, %entry.omp.precond.end_crit_edge ], [ %i.0.lcssa, %DIR.QUAL.LIST.END.4 ]
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %.pre-phi) #2
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %1) #2
  %add16 = add nsw i64 %7, %i.1
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %0) #2
  ret i64 %add16
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang a53756907774b7d85a523756d285be3e3ac08d1c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 309bff2407d20783eb5e0b1eee132fe0caa366ba)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"long", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !8, i64 0}
!7 = !{!"array@_ZTSA101_f", !8, i64 0}
!8 = !{!"float", !4, i64 0}

