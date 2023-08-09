; RUN: opt -O3 -paropt=31 -S %s | FileCheck %s

; Test src:
;
; void test1(int *P, int N) {
; #pragma omp parallel for
;   for (int I = 0; I < N; ++I)
; #pragma omp task
;     P[I] = I;
; }

; Check that shared P is not privatized in the 'parallel for'. Privatization
; is not legal because P is captured by a nested task.

; CHECK: define dso_local void @test1(ptr noundef %P, i32 noundef %N) {{.*}} {
; CHECK:   [[PADDR:%.+]] = alloca ptr
; CHECK:   store ptr %P, ptr [[PADDR]]
; CHECK:   call {{.+}} @__kmpc_fork_call({{.+}}, i32 3, ptr nonnull @{{.+}}, ptr nonnull [[PADDR]], i64 0, i64 %{{.+}})
; CHECK: }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @test1(ptr noundef %P, i32 noundef %N) #0 {
entry:
  %P.addr = alloca ptr, align 8
  %N.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store ptr %P, ptr %P.addr, align 8, !tbaa !4
  store i32 %N, ptr %N.addr, align 4, !tbaa !8
  call void @llvm.lifetime.start.p0(i64 4, ptr %.capture_expr.0) #2
  %0 = load i32, ptr %N.addr, align 4, !tbaa !8
  store i32 %0, ptr %.capture_expr.0, align 4, !tbaa !8
  call void @llvm.lifetime.start.p0(i64 4, ptr %.capture_expr.1) #2
  %1 = load i32, ptr %.capture_expr.0, align 4, !tbaa !8
  %sub = sub nsw i32 %1, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.1, align 4, !tbaa !8
  %2 = load i32, ptr %.capture_expr.0, align 4, !tbaa !8
  %cmp = icmp slt i32 0, %2
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv) #2
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.lb) #2
  store i32 0, ptr %.omp.lb, align 4, !tbaa !8
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub) #2
  %3 = load i32, ptr %.capture_expr.1, align 4, !tbaa !8
  store i32 %3, ptr %.omp.ub, align 4, !tbaa !8
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %P.addr, ptr null, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %I, i32 0, i32 1) ]
  %5 = load i32, ptr %.omp.lb, align 4, !tbaa !8
  store i32 %5, ptr %.omp.iv, align 4, !tbaa !8
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %6 = load i32, ptr %.omp.iv, align 4, !tbaa !8
  %7 = load i32, ptr %.omp.ub, align 4, !tbaa !8
  %cmp3 = icmp sle i32 %6, %7
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start.p0(i64 4, ptr %I) #2
  %8 = load i32, ptr %.omp.iv, align 4, !tbaa !8
  %mul = mul nsw i32 %8, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, ptr %I, align 4, !tbaa !8
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %I, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %P.addr, ptr null, i32 1) ]
  %10 = load i32, ptr %I, align 4, !tbaa !8
  %11 = load ptr, ptr %P.addr, align 8, !tbaa !4
  %12 = load i32, ptr %I, align 4, !tbaa !8
  %idxprom = sext i32 %12 to i64
  %arrayidx = getelementptr inbounds i32, ptr %11, i64 %idxprom
  store i32 %10, ptr %arrayidx, align 4, !tbaa !8
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.TASK"() ]
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  call void @llvm.lifetime.end.p0(i64 4, ptr %I) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %13 = load i32, ptr %.omp.iv, align 4, !tbaa !8
  %add5 = add nsw i32 %13, 1
  store i32 %add5, ptr %.omp.iv, align 4, !tbaa !8
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.lb) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.capture_expr.1) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.capture_expr.0) #2
  ret void
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!4 = !{!5, !5, i64 0}
!5 = !{!"pointer@_ZTSPi", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !6, i64 0}
