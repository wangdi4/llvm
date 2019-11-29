; RUN: opt < %s -vpo-paropt -S | FileCheck %s
; RUN: opt < %s -passes='vpo-paropt'  -S | FileCheck %s

; Original code:
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
entry:
  %i = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #2
  %1 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1) #2
  %2 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !2
  %3 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %3) #2
  store volatile i32 99, i32* %.omp.ub, align 4, !tbaa !2
  %end.dir.temp = alloca i1, align 1
  br label %DIR.OMP.LOOP.1

DIR.OMP.LOOP.1:                                   ; preds = %entry
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]
  br label %DIR.OMP.LOOP.27

DIR.OMP.LOOP.27:                                  ; preds = %DIR.OMP.LOOP.1
  %temp.load = load volatile i1, i1* %end.dir.temp, align 1
  br i1 %temp.load, label %omp.loop.exit.split, label %DIR.OMP.LOOP.2

DIR.OMP.LOOP.2:                                   ; preds = %DIR.OMP.LOOP.27
  %5 = load i32, i32* %.omp.lb, align 4, !tbaa !2
  store volatile i32 %5, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.LOOP.2
  %l.0 = phi i32 [ 0, %DIR.OMP.LOOP.2 ], [ %inc, %omp.inner.for.body ]
  %6 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !2
  %7 = load volatile i32, i32* %.omp.ub, align 4, !tbaa !2
  %cmp = icmp sgt i32 %6, %7
  br i1 %cmp, label %omp.loop.exit.split, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !2
  store i32 %8, i32* %i, align 4, !tbaa !2
  %inc = add nuw nsw i32 %l.0, 1
  %9 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !2
  %add1 = add nsw i32 %9, 1
  store volatile i32 %add1, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.loop.exit.split:                              ; preds = %omp.inner.for.cond, %DIR.OMP.LOOP.27
  %l.1 = phi i32 [ 0, %DIR.OMP.LOOP.27 ], [ %l.0, %omp.inner.for.cond ]
  br label %DIR.OMP.END.LOOP.3

DIR.OMP.END.LOOP.3:                               ; preds = %omp.loop.exit.split
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.LOOP"() ]
  br label %DIR.OMP.END.LOOP.4

DIR.OMP.END.LOOP.4:                               ; preds = %DIR.OMP.END.LOOP.3
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %3) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %2) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #2
  ret i32 %l.1
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
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
