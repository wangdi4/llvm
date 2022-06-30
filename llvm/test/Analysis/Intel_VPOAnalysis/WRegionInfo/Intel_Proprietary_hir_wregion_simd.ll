; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,asserts
;
; RUN: opt -enable-new-pm=0 -hir-framework -vpo-wrncollection -vpo-wrninfo -hir-vplan-vec -debug-only=vpo-wrninfo %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-vplan-vec,print<hir-framework>,print<vpo-wrncollection>,require<vpo-wrninfo>" -debug-only=vpo-wrninfo %s 2>&1 | FileCheck %s

; WARNING!!!
; WARNING!!!      ** CONTAINS INTEL IP **
; WARNING!!!      DO NOT SHARE EXTERNALLY
; WARNING!!!

; Test src:
;
; void foo(int *a, int step) {
;   int x = 10;
;   int n = 2;
; #pragma omp simd linear(n : step) lastprivate(x)
;   for (int i = 0; i < 100; i++) {
;     x = i;
;     a[i] = x;
;     n += step;
;   }
; }

; Check for wregion-collection's prints to ensure clauses from HIR were parsed.

; CHECK: PRIVATE clause (size=1): (&((LINEAR ptr %x.priv)[i64 0]) {{.*}}) , TYPED (TYPE: i32, NUM_ELEMENTS: i32 1)
; CHECK: LINEAR clause (size=2): (&((LINEAR ptr %n.linear)[i64 0])
; CHECK-SAME: LINEAR i32 %step

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(ptr noundef %a, i32 noundef %step) local_unnamed_addr #0 {
DIR.OMP.SIMD.28:
  %n.linear = alloca i32, align 4
  %i.linear.iv = alloca i32, align 4
  %x.priv = alloca i32, align 4
  %x = alloca i32, align 4
  %n = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %x) #2
  store i32 10, ptr %x, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %n) #2
  store i32 2, ptr %n, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %.omp.iv) #2
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %.omp.ub) #2
  store i32 99, ptr %.omp.ub, align 4, !tbaa !4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.28
  %0 = load i32, ptr %n, align 4
  store i32 %0, ptr %n.linear, align 4
  br label %DIR.OMP.SIMD.1.split13

DIR.OMP.SIMD.1.split13:                           ; preds = %DIR.OMP.SIMD.1
  br label %DIR.OMP.SIMD.1.split

DIR.OMP.SIMD.1.split:                             ; preds = %DIR.OMP.SIMD.1.split13
  %1 = load i32, ptr %.omp.ub, align 4
  br label %DIR.OMP.SIMD.29

DIR.OMP.SIMD.29:                                  ; preds = %DIR.OMP.SIMD.1.split
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.29
  %cmp.not11 = icmp sgt i32 0, %1
  br i1 %cmp.not11, label %DIR.OMP.END.SIMD.4.loopexit, label %omp.inner.for.body.lr.ph

omp.inner.for.body.lr.ph:                         ; preds = %DIR.OMP.SIMD.2
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.LINEAR:TYPED"(ptr %n.linear, i32 0, i32 1, i32 %step),
    "QUAL.OMP.LASTPRIVATE:TYPED"(ptr null, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %x.priv, i32 0, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body.lr.ph, %omp.inner.for.body
  %.omp.iv.local.012 = phi i32 [ 0, %omp.inner.for.body.lr.ph ], [ %add2, %omp.inner.for.body ]
  call void @llvm.lifetime.start.p0(i64 4, ptr %i.linear.iv) #2, !llvm.access.group !8
  store i32 %.omp.iv.local.012, ptr %i.linear.iv, align 4, !tbaa !4, !llvm.access.group !8
  store i32 %.omp.iv.local.012, ptr %x.priv, align 4, !tbaa !4, !llvm.access.group !8
  %3 = load i32, ptr %i.linear.iv, align 4, !tbaa !4, !llvm.access.group !8
  %idxprom = sext i32 %3 to i64
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %idxprom
  store i32 %.omp.iv.local.012, ptr %arrayidx, align 4, !tbaa !4, !llvm.access.group !8
  %4 = load i32, ptr %n.linear, align 4, !tbaa !4, !llvm.access.group !8
  %add1 = add nsw i32 %4, %step
  store i32 %add1, ptr %n.linear, align 4, !tbaa !4, !llvm.access.group !8
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i.linear.iv) #2, !llvm.access.group !8
  %add2 = add nsw i32 %.omp.iv.local.012, 1
  %5 = add i32 %1, 1
  %cmp.not = icmp sgt i32 %5, %add2
  br i1 %cmp.not, label %omp.inner.for.body, label %omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge, !llvm.loop !9

omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge: ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SIMD"() ]
  %6 = load i32, ptr %n.linear, align 4
  store i32 %6, ptr %n, align 4
  br label %DIR.OMP.END.SIMD.4.loopexit

DIR.OMP.END.SIMD.4.loopexit:                      ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge, %DIR.OMP.SIMD.2
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %DIR.OMP.END.SIMD.4.loopexit
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.4
  br label %DIR.OMP.END.SIMD.410

DIR.OMP.END.SIMD.410:                             ; preds = %DIR.OMP.END.SIMD.3
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.ub) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.iv) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %n) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %x) #2
  ret void
}

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly mustprogress nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}
!nvvm.annotations = !{}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = distinct !{}
!9 = distinct !{!9, !10, !11, !12}
!10 = !{!"llvm.loop.vectorize.enable", i1 true}
!11 = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}
!12 = !{!"llvm.loop.parallel_accesses", !8}
; end INTEL_FEATURE_SW_ADVANCED
