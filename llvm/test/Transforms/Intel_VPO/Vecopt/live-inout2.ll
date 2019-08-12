; REQUIRES: asserts
; RUN: opt %s -VPlanDriver -vplan-force-build -loopopt=0 -debug -vplan-plain-dump=true -vplan-dump-liveness=1 -disable-vplan-codegen 2>&1 | FileCheck %s 

; verify live-in and live-out analysis
; CHECK: Live-in and Live-out info:
; CHECK-NEXT: External defs:
; CHECK-DAG: i64 %ret.promoted
; CHECK-DAG: i64 %3
; CHECK-DAG: i64* %k
; CHECK-DAG: i64* %lb
; CHECK-DAG: i64* %ub
; CHECK-DAG: [101 x float]* %B
; CHECK-DAG: [101 x float]* %a
; CHECK-DAG: i8* %4
; CHECK-NEXT: Used externally:
; CHECK-DAG: i64 {{%vp.*}}
; CHECK-DAG: i64 {{%vp.*}}
; CHECK-DAG: i64 {{%vp.*}}
; CHECK-DAG: i64 %ret.promoted livein in the loops:  BB2
; CHECK-DAG: i64 {{%vp.*}} liveout in the loop: BB2
; CHECK-DAG: i8* %4 livein in the loops:  BB2
; CHECK-DAG: i64* %k livein in the loops:  BB2
; CHECK-DAG: i64* %ub livein in the loops:  BB2
; CHECK-DAG: [101 x float]* %a livein in the loops:  BB4 BB2
; CHECK-DAG: i64 {{%vp.*}} livein in the loops:  BB4
; CHECK-DAG: i64 {{%vp.*}} liveout in the loop: BB4
; CHECK-DAG: i64 {{%vp.*}} livein in the loops:  BB4
; CHECK-DAG: i64* %k livein in the loops:  BB2
; CHECK-DAG: i64* %lb livein in the loops:  BB2
; CHECK-DAG: i64 {{%vp.*}} livein in the loops:  BB9
; CHECK-DAG: [101 x float]* %B livein in the loops:  BB9 BB2
; CHECK-DAG: i64 {{%vp.*}} livein in the loops:  BB9
; CHECK-DAG: i64 {{%vp.*}} liveout in the loop: BB9
; CHECK-DAG: i64 {{%vp.*}} livein in the loops:  BB9
; CHECK-DAG: i64* %k livein in the loops:  BB2
; CHECK-DAG: [101 x float]* %B livein in the loops:  BB2
; CHECK-DAG: i64 {{%vp.*}} liveout in the loop: BB2
; CHECK-DAG: i8* %4 livein in the loops:  BB2
; CHECK-DAG: i64 {{%vp.*}} liveout in the loop: BB2
; CHECK-DAG: i64 %3 livein in the loops:  BB2
; CHECK-NEXT: Live-in and Live-out info end

; Function Attrs: nounwind uwtable
define dso_local i64 @_Z3foolPlS_PA101_fS1_(i64 %n, i64* nocapture readonly %lb, i64* nocapture readonly %ub, [101 x float]* nocapture readonly %a, [101 x float]* nocapture %B) local_unnamed_addr #0 {
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
  %.pre58 = bitcast i64* %.omp.ub to i8*
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
  %cmp453 = icmp slt i64 %3, 0
  br i1 %cmp453, label %omp.precond.then.omp.loop.exit_crit_edge, label %omp.inner.for.body.lr.ph

omp.precond.then.omp.loop.exit_crit_edge:         ; preds = %DIR.QUAL.LIST.END.2
  %.pre.pre = load i64, i64* %ret, align 8, !tbaa !2
  br label %omp.loop.exit

omp.inner.for.body.lr.ph:                         ; preds = %DIR.QUAL.LIST.END.2
  %4 = bitcast i64* %k to i8*
  %ret.promoted = load i64, i64* %ret, align 8, !tbaa !2
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %for.end22, %omp.inner.for.body.lr.ph
  %add2655 = phi i64 [ %ret.promoted, %omp.inner.for.body.lr.ph ], [ %add26, %for.end22 ]
  %storemerge54 = phi i64 [ 0, %omp.inner.for.body.lr.ph ], [ %add27, %for.end22 ]
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %4) #2
  store i64 0, i64* %k, align 8, !tbaa !2
  %arrayidx = getelementptr inbounds i64, i64* %ub, i64 %storemerge54
  %5 = load i64, i64* %arrayidx, align 8, !tbaa !2
  %cmp646 = icmp sgt i64 %5, 0
  br i1 %cmp646, label %for.body.preheader, label %for.cond11.preheader

for.body.preheader:                               ; preds = %omp.inner.for.body
  br label %for.body

for.cond.for.cond11.preheader_crit_edge:          ; preds = %for.body
  %conv10.lcssa = phi i64 [ %conv10, %for.body ]
  store i64 %conv10.lcssa, i64* %k, align 8, !tbaa !2
  br label %for.cond11.preheader

for.cond11.preheader:                             ; preds = %for.cond.for.cond11.preheader_crit_edge, %omp.inner.for.body
  %k.promoted51 = phi i64 [ %conv10.lcssa, %for.cond.for.cond11.preheader_crit_edge ], [ 0, %omp.inner.for.body ]
  %arrayidx12 = getelementptr inbounds i64, i64* %lb, i64 %storemerge54
  %6 = load i64, i64* %arrayidx12, align 8, !tbaa !2
  %cmp1349 = icmp sgt i64 %6, 0
  br i1 %cmp1349, label %for.body14.preheader, label %for.end22

for.body14.preheader:                             ; preds = %for.cond11.preheader
  br label %for.body14

for.body:                                         ; preds = %for.body.preheader, %for.body
  %conv1048 = phi i64 [ %conv10, %for.body ], [ 0, %for.body.preheader ]
  %j.047 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx8 = getelementptr inbounds [101 x float], [101 x float]* %a, i64 %storemerge54, i64 %j.047
  %7 = load float, float* %arrayidx8, align 4, !tbaa !6
  %conv = sitofp i64 %conv1048 to float
  %add9 = fadd float %7, %conv
  %conv10 = fptosi float %add9 to i64
  %inc = add nuw nsw i64 %j.047, 1
  %cmp6 = icmp slt i64 %inc, %5
  br i1 %cmp6, label %for.body, label %for.cond.for.cond11.preheader_crit_edge

for.body14:                                       ; preds = %for.body14.preheader, %for.body14
  %conv1952 = phi i64 [ %conv19, %for.body14 ], [ %k.promoted51, %for.body14.preheader ]
  %j.150 = phi i64 [ %inc21, %for.body14 ], [ 0, %for.body14.preheader ]
  %arrayidx16 = getelementptr inbounds [101 x float], [101 x float]* %B, i64 %storemerge54, i64 %j.150
  %8 = load float, float* %arrayidx16, align 4, !tbaa !6
  %conv17 = sitofp i64 %conv1952 to float
  %add18 = fadd float %8, %conv17
  %conv19 = fptosi float %add18 to i64
  %inc21 = add nuw nsw i64 %j.150, 1
  %cmp13 = icmp slt i64 %inc21, %6
  br i1 %cmp13, label %for.body14, label %for.cond11.for.end22_crit_edge

for.cond11.for.end22_crit_edge:                   ; preds = %for.body14
  %conv19.lcssa = phi i64 [ %conv19, %for.body14 ]
  store i64 %conv19.lcssa, i64* %k, align 8, !tbaa !2
  br label %for.end22

for.end22:                                        ; preds = %for.cond11.for.end22_crit_edge, %for.cond11.preheader
  %9 = phi i64 [ %conv19.lcssa, %for.cond11.for.end22_crit_edge ], [ %k.promoted51, %for.cond11.preheader ]
  %conv23 = sitofp i64 %9 to float
  %arrayidx25 = getelementptr inbounds [101 x float], [101 x float]* %B, i64 %storemerge54, i64 %storemerge54
  store float %conv23, float* %arrayidx25, align 4, !tbaa !6
  %add26 = add nsw i64 %add2655, %9
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %4) #2
  %add27 = add nuw nsw i64 %storemerge54, 1
  %cmp4 = icmp slt i64 %storemerge54, %3
  br i1 %cmp4, label %omp.inner.for.body, label %omp.inner.for.cond.omp.loop.exit_crit_edge

omp.inner.for.cond.omp.loop.exit_crit_edge:       ; preds = %for.end22
  %add26.lcssa = phi i64 [ %add26, %for.end22 ]
  %add27.lcssa = phi i64 [ %add27, %for.end22 ]
  %storemerge54.lcssa = phi i64 [ %storemerge54, %for.end22 ]
  store i64 %add26.lcssa, i64* %ret, align 8, !tbaa !2
  store i64 %add27.lcssa, i64* %.omp.iv, align 8, !tbaa !2
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.precond.then.omp.loop.exit_crit_edge, %omp.inner.for.cond.omp.loop.exit_crit_edge
  %.pre = phi i64 [ %add26.lcssa, %omp.inner.for.cond.omp.loop.exit_crit_edge ], [ %.pre.pre, %omp.precond.then.omp.loop.exit_crit_edge ]
  %i.0.lcssa = phi i64 [ %storemerge54.lcssa, %omp.inner.for.cond.omp.loop.exit_crit_edge ], [ undef, %omp.precond.then.omp.loop.exit_crit_edge ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %omp.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %entry.omp.precond.end_crit_edge, %DIR.OMP.END.SIMD.3
  %.pre-phi = phi i8* [ %.pre58, %entry.omp.precond.end_crit_edge ], [ %2, %DIR.OMP.END.SIMD.3 ]
  %10 = phi i64 [ 0, %entry.omp.precond.end_crit_edge ], [ %.pre, %DIR.OMP.END.SIMD.3 ]
  %i.1 = phi i64 [ undef, %entry.omp.precond.end_crit_edge ], [ %i.0.lcssa, %DIR.OMP.END.SIMD.3 ]
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %.pre-phi) #2
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %1) #2
  %add28 = add nsw i64 %10, %i.1
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %0) #2
  ret i64 %add28
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

