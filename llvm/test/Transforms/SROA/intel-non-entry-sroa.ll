; RUN: opt -passes=sroa -S < %s | FileCheck %s

; SROA is modified to handle blocks preceding a SIMD directive (SIMD loop
; preheader)
; Check that %tempor.priv is completely removed after SROA.

; CHECK-NOT: tempor.priv

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @_Z3fooPff(float* nocapture %p, float %init) local_unnamed_addr #0 {
entry:
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tempor = alloca float, align 4
  %0 = load float, float* %p, align 4, !tbaa !2
  %cmp = fcmp ogt float %0, 2.560000e+02
  br i1 %cmp, label %return, label %if.end

if.end:                                           ; preds = %entry
  %1 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1) #2
  %2 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2) #2
  store i32 99, i32* %.omp.ub, align 4, !tbaa !6
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %if.end
  %tempor.priv = alloca float, align 4
  br label %DIR.OMP.SIMD.1.split16

DIR.OMP.SIMD.1.split16:                           ; preds = %DIR.OMP.SIMD.1
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.PRIVATE"(float* %tempor.priv) ]
  br label %DIR.OMP.SIMD.1.split

DIR.OMP.SIMD.1.split:                             ; preds = %DIR.OMP.SIMD.1.split16
  %4 = load i32, i32* %.omp.ub
  br label %DIR.OMP.SIMD.213

DIR.OMP.SIMD.213:                                 ; preds = %DIR.OMP.SIMD.1.split
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.213
  %cmp114 = icmp sgt i32 0, %4
  br i1 %cmp114, label %omp.loop.exit.split, label %omp.inner.for.body.lr.ph

omp.inner.for.body.lr.ph:                         ; preds = %DIR.OMP.SIMD.2
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %if.end5, %omp.inner.for.body.lr.ph
  %.omp.iv.local.015 = phi i32 [ 0, %omp.inner.for.body.lr.ph ], [ %add6, %if.end5 ]
  %5 = bitcast float* %tempor.priv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #2
  %conv = sitofp i32 %.omp.iv.local.015 to float
  %add2 = fadd float %conv, %init
  store float %add2, float* %tempor.priv, align 4, !tbaa !2
  %and = and i32 %.omp.iv.local.015, 2
  %tobool = icmp eq i32 %and, 0
  br i1 %tobool, label %if.end5, label %if.then3

if.then3:                                         ; preds = %omp.inner.for.body
  %idxprom = sext i32 %.omp.iv.local.015 to i64
  %arrayidx = getelementptr inbounds float, float* %p, i64 %idxprom
  %6 = load float, float* %arrayidx, align 4, !tbaa !2
  %add4 = fadd float %6, %add2
  store float %add4, float* %arrayidx, align 4, !tbaa !2
  br label %if.end5

if.end5:                                          ; preds = %omp.inner.for.body, %if.then3
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %5) #2
  %add6 = add nsw i32 %.omp.iv.local.015, 1
  %7 = add i32 %4, 1
  %cmp1 = icmp sgt i32 %7, %add6
  br i1 %cmp1, label %omp.inner.for.body, label %omp.inner.for.cond.omp.loop.exit.split_crit_edge

omp.inner.for.cond.omp.loop.exit.split_crit_edge: ; preds = %if.end5
  br label %omp.loop.exit.split

omp.loop.exit.split:                              ; preds = %omp.inner.for.cond.omp.loop.exit.split_crit_edge, %DIR.OMP.SIMD.2
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %omp.loop.exit.split
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %DIR.OMP.END.SIMD.3
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %2) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1) #2
  br label %return

return:                                           ; preds = %entry, %DIR.OMP.END.SIMD.4
  ret void
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}
