; REQUIRES: asserts
; RUN: opt %s -VPlanDriver -vplan-force-build -loopopt=0 -debug -vplan-plain-dump=true -vplan-dump-liveness=1 -disable-vplan-codegen 2>&1 | FileCheck %s 

target triple = "x86_64-unknown-linux-gnu"

; verify live-in and live-out analysis
; CHECK:Live-in and Live-out info:
; CHECK-NEXT:External defs:
; CHECK-DAG: i64* %lb
; CHECK-DAG: [101 x float]* %a
; CHECK-DAG: i64* %ub
; CHECK-DAG: i64 %2
; CHECK-NEXT:Used externally:
; CHECK-DAG: i64 {{%vp.*}}
; CHECK-DAG: i64 {{%vp.*}}
; CHECK-DAG: i64 {{%vp.*}}
; CHECK-DAG: i64* %ub livein in the loops:  BB2
; CHECK-DAG: i64 {{%vp.*}} liveout in the loop: BB2
; CHECK-DAG: i64 {{%vp.*}} liveout in the loop: BB2
; CHECK-DAG: [101 x float]* %a livein in the loops:  BB2
; CHECK-DAG: i64* %lb livein in the loops:  BB2
; CHECK-DAG: i64 {{%vp.*}} liveout in the loop: BB2
; CHECK-DAG: i64 %2 livein in the loops:  BB2
; CHECK-NEXT:Live-in and Live-out info end

; Function Attrs: nounwind uwtable
define dso_local i64 @_Z3foolPlS_PA101_f(i64 %n, i64* nocapture %lb, i64* nocapture readonly %ub, [101 x float]* nocapture readonly %a) local_unnamed_addr #0 {
entry:
  %.omp.iv = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %0 = bitcast i64* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %0) #2
  %cmp = icmp sgt i64 %n, 0
  br i1 %cmp, label %omp.precond.then, label %entry.omp.precond.end_crit_edge

entry.omp.precond.end_crit_edge:                  ; preds = %entry
  %.pre = bitcast i64* %.omp.ub to i8*
  br label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %sub2 = add nsw i64 %n, -1
  %1 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %1) #2
  store i64 %sub2, i64* %.omp.ub, align 8, !tbaa !2
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.precond.then
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i64* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i64* %.omp.ub) ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %DIR.OMP.SIMD.1
  store i64 0, i64* %.omp.iv, align 8, !tbaa !2
  %2 = load i64, i64* %.omp.ub, align 8, !tbaa !2
  %cmp430 = icmp slt i64 %2, 0
  br i1 %cmp430, label %omp.loop.exit, label %omp.inner.for.body.preheader

omp.inner.for.body.preheader:                     ; preds = %DIR.QUAL.LIST.END.2
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body.preheader, %omp.inner.for.body
  %ret.033 = phi i64 [ %add7, %omp.inner.for.body ], [ 0, %omp.inner.for.body.preheader ]
  %k.032 = phi i64 [ %add9, %omp.inner.for.body ], [ 1, %omp.inner.for.body.preheader ]
  %storemerge31 = phi i64 [ %add13, %omp.inner.for.body ], [ 0, %omp.inner.for.body.preheader ]
  %arrayidx = getelementptr inbounds i64, i64* %ub, i64 %storemerge31
  %3 = load i64, i64* %arrayidx, align 8, !tbaa !2
  %cmp6 = icmp sgt i64 %3, 0
  %conv = zext i1 %cmp6 to i64
  %add7 = add nuw nsw i64 %ret.033, %conv
  %add9 = add nsw i64 %3, %k.032
  %arrayidx11 = getelementptr inbounds [101 x float], [101 x float]* %a, i64 %storemerge31, i64 %storemerge31
  %4 = load float, float* %arrayidx11, align 4, !tbaa !6
  %conv12 = fptosi float %4 to i64
  store i64 %conv12, i64* %lb, align 8, !tbaa !2
  %add13 = add nuw nsw i64 %storemerge31, 1
  %cmp4 = icmp slt i64 %storemerge31, %2
  br i1 %cmp4, label %omp.inner.for.body, label %omp.inner.for.cond.omp.loop.exit_crit_edge

omp.inner.for.cond.omp.loop.exit_crit_edge:       ; preds = %omp.inner.for.body
  %add7.lcssa = phi i64 [ %add7, %omp.inner.for.body ]
  %add9.lcssa = phi i64 [ %add9, %omp.inner.for.body ]
  %add13.lcssa = phi i64 [ %add13, %omp.inner.for.body ]
  store i64 %add13.lcssa, i64* %.omp.iv, align 8, !tbaa !2
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.cond.omp.loop.exit_crit_edge, %DIR.QUAL.LIST.END.2
  %k.0.lcssa = phi i64 [ %add9.lcssa, %omp.inner.for.cond.omp.loop.exit_crit_edge ], [ 1, %DIR.QUAL.LIST.END.2 ]
  %ret.0.lcssa = phi i64 [ %add7.lcssa, %omp.inner.for.cond.omp.loop.exit_crit_edge ], [ 0, %DIR.QUAL.LIST.END.2 ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %omp.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %entry.omp.precond.end_crit_edge, %DIR.OMP.END.SIMD.3
  %.pre-phi = phi i8* [ %.pre, %entry.omp.precond.end_crit_edge ], [ %1, %DIR.OMP.END.SIMD.3 ]
  %k.1 = phi i64 [ 1, %entry.omp.precond.end_crit_edge ], [ %k.0.lcssa, %DIR.OMP.END.SIMD.3 ]
  %ret.1 = phi i64 [ 0, %entry.omp.precond.end_crit_edge ], [ %ret.0.lcssa, %DIR.OMP.END.SIMD.3 ]
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %.pre-phi) #2
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %0) #2
  %add14 = add nsw i64 %ret.1, %k.1
  %5 = load i64, i64* %lb, align 8, !tbaa !2
  %add15 = add nsw i64 %add14, %5
  ret i64 %add15
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

