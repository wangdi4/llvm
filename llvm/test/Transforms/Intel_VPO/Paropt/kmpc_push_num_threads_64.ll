; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; void foo(float **x, long long int n) {
; #pragma omp parallel for num_threads(n)
;   for (long long int i = -10; i < n; ++i)
;     x[i] = 0;
; }

; Verify calling convention of __kmpc_push_num_threads
; CHECK: declare void @__kmpc_push_num_threads({{[^,]+}}, i32, i32)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(ptr noundef %x, i64 noundef %n) #0 {
entry:
  %x.addr = alloca ptr, align 8
  %n.addr = alloca i64, align 8
  %tmp = alloca i64, align 8
  %.capture_expr.0 = alloca i64, align 8
  %.capture_expr.1 = alloca i64, align 8
  %.omp.iv = alloca i64, align 8
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %i = alloca i64, align 8
  store ptr %x, ptr %x.addr, align 8, !tbaa !4
  store i64 %n, ptr %n.addr, align 8, !tbaa !8
  call void @llvm.lifetime.start.p0(i64 8, ptr %.capture_expr.0) #2
  %0 = load i64, ptr %n.addr, align 8, !tbaa !8
  store i64 %0, ptr %.capture_expr.0, align 8, !tbaa !8
  call void @llvm.lifetime.start.p0(i64 8, ptr %.capture_expr.1) #2
  %1 = load i64, ptr %.capture_expr.0, align 8, !tbaa !8
  %sub = sub i64 %1, -10
  %div = udiv i64 %sub, 1
  %sub1 = sub i64 %div, 1
  store i64 %sub1, ptr %.capture_expr.1, align 8, !tbaa !8
  %2 = load i64, ptr %.capture_expr.0, align 8, !tbaa !8
  %cmp = icmp slt i64 -10, %2
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  call void @llvm.lifetime.start.p0(i64 8, ptr %.omp.iv) #2
  call void @llvm.lifetime.start.p0(i64 8, ptr %.omp.lb) #2
  store i64 0, ptr %.omp.lb, align 8, !tbaa !8
  call void @llvm.lifetime.start.p0(i64 8, ptr %.omp.ub) #2
  %3 = load i64, ptr %.capture_expr.1, align 8, !tbaa !8
  store i64 %3, ptr %.omp.ub, align 8, !tbaa !8
  %4 = load i64, ptr %n.addr, align 8, !tbaa !8
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.NUM_THREADS"(i64 %4),
    "QUAL.OMP.SHARED:TYPED"(ptr %x.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %n.addr, i64 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i64 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i64 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i64 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i64 0, i32 1) ]
  %6 = load i64, ptr %.omp.lb, align 8, !tbaa !8
  store i64 %6, ptr %.omp.iv, align 8, !tbaa !8
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %7 = load i64, ptr %.omp.iv, align 8, !tbaa !8
  %8 = load i64, ptr %.omp.ub, align 8, !tbaa !8
  %add = add i64 %8, 1
  %cmp2 = icmp ult i64 %7, %add
  br i1 %cmp2, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start.p0(i64 8, ptr %i) #2
  %9 = load i64, ptr %.omp.iv, align 8, !tbaa !8
  %mul = mul i64 %9, 1
  %add3 = add i64 -10, %mul
  store i64 %add3, ptr %i, align 8, !tbaa !8
  %10 = load ptr, ptr %x.addr, align 8, !tbaa !4
  %11 = load i64, ptr %i, align 8, !tbaa !8
  %arrayidx = getelementptr inbounds ptr, ptr %10, i64 %11
  store ptr null, ptr %arrayidx, align 8, !tbaa !10
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  call void @llvm.lifetime.end.p0(i64 8, ptr %i) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i64, ptr %.omp.iv, align 8, !tbaa !8
  %add4 = add nuw i64 %12, 1
  store i64 %add4, ptr %.omp.iv, align 8, !tbaa !8
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  call void @llvm.lifetime.end.p0(i64 8, ptr %.omp.ub) #2
  call void @llvm.lifetime.end.p0(i64 8, ptr %.omp.lb) #2
  call void @llvm.lifetime.end.p0(i64 8, ptr %.omp.iv) #2
  call void @llvm.lifetime.end.p0(i64 8, ptr %.capture_expr.1) #2
  call void @llvm.lifetime.end.p0(i64 8, ptr %.capture_expr.0) #2
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
!5 = !{!"pointer@_ZTSPPf", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !9, i64 0}
!9 = !{!"long long", !6, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"pointer@_ZTSPf", !6, i64 0}
