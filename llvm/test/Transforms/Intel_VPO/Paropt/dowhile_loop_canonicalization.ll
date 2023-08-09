; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; int foo() {
;   int i, l = 0;
; #pragma omp for
;   for (i = 0; i < 100; ++i)
;     ++l;
;
;   return l;
; }

; fixOmpDoWhileLoopImpl() used to fail to canonicalize OpenMP do-while
; loops with multiple PHINodes inside the loop body feeding and increment
; operation.  This test is actually checking that there is no compilation fail.
; The IR below was collected right before VPO Paropt Pass.

; Try to check that the ordering of PHINodes in the loop is such that
; it would cause an assertion with the old code in fixOmpDoWhileLoopImpl():
; CHECK: [[L:%[a-zA-Z._0-9]+]] = phi i32 [ 0, {{.*}} ], [ [[INC:%[a-zA-Z._0-9]+]], {{.*}} ]
; CHECK: [[IV:%[a-zA-Z._0-9]+]] = phi i32 [ [[ADD:%[a-zA-Z._0-9]+]], {{.*}} ]
; CHECK-DAG: [[INC]] = add nuw nsw i32 [[L]], 1
; CHECK-DAG: [[ADD]] = add nsw i32 [[IV]], 1
; CHECK: [[CMP:%[a-zA-Z._0-9]+]] = icmp sle i32 [[ADD]]
; CHECK br i1 [[CMP]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @foo() local_unnamed_addr #0 {
DIR.OMP.LOOP.26:
  %i = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i) #2
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %.omp.iv) #2
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %.omp.lb) #2
  store i32 0, ptr %.omp.lb, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %.omp.ub) #2
  store volatile i32 99, ptr %.omp.ub, align 4, !tbaa !4
  %end.dir.temp = alloca i1, align 1
  br label %DIR.OMP.LOOP.1

DIR.OMP.LOOP.1:                                   ; preds = %DIR.OMP.LOOP.26
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]
  br label %DIR.OMP.LOOP.27

DIR.OMP.LOOP.27:                                  ; preds = %DIR.OMP.LOOP.1
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.LOOP.4, label %DIR.OMP.LOOP.2

DIR.OMP.LOOP.2:                                   ; preds = %DIR.OMP.LOOP.27
  %1 = load i32, ptr %.omp.lb, align 4, !tbaa !4
  store volatile i32 %1, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.LOOP.2
  %l.0 = phi i32 [ 0, %DIR.OMP.LOOP.2 ], [ %inc, %omp.inner.for.body ]
  %2 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !4
  %3 = load volatile i32, ptr %.omp.ub, align 4, !tbaa !4
  %cmp.not = icmp sgt i32 %2, %3
  br i1 %cmp.not, label %DIR.OMP.END.LOOP.4.loopexit, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !4
  store i32 %4, ptr %i, align 4, !tbaa !4
  %inc = add nuw nsw i32 %l.0, 1
  %5 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !4
  %add1 = add nsw i32 %5, 1
  store volatile i32 %add1, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

DIR.OMP.END.LOOP.4.loopexit:                      ; preds = %omp.inner.for.cond
  br label %DIR.OMP.END.LOOP.4

DIR.OMP.END.LOOP.4:                               ; preds = %DIR.OMP.END.LOOP.4.loopexit, %DIR.OMP.LOOP.27
  %l.1 = phi i32 [ 0, %DIR.OMP.LOOP.27 ], [ %l.0, %DIR.OMP.END.LOOP.4.loopexit ]
  br label %DIR.OMP.END.LOOP.3

DIR.OMP.END.LOOP.3:                               ; preds = %DIR.OMP.END.LOOP.4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]
  br label %DIR.OMP.END.LOOP.48

DIR.OMP.END.LOOP.48:                              ; preds = %DIR.OMP.END.LOOP.3
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.ub) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.lb) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.iv) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i) #2
  ret i32 %l.1
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

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
