; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; void foo(float *x, long long int n) {
; #pragma omp parallel for num_threads(n)
;   for (long long int i = 0; i < n; ++i)
;     x[i] = 0;
; }

; Verify that all arguments of the outlined function are pointers.
; If num_threads value is not removed from the clause, it will
; appear as a non-pointer argument.
; CHECK: define internal void @_Z3fooPfx.DIR.OMP.PARALLEL.LOOP.{{[0-9]+}}.{{.*}}(ptr {{[^,]*}}, ptr {{[^,]*}}, ptr {{[^,]*}}, ptr {{[^,]*}}, ptr {{[^,]*}}, ptr {{[^,]*}})

target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @_Z3fooPfx(ptr %x, i64 %n) #0 {
entry:
  %x.addr = alloca ptr, align 4
  %n.addr = alloca i64, align 8
  %.omp.iv = alloca i64, align 8
  %tmp = alloca i64, align 4
  %.capture_expr. = alloca i64, align 8
  %.capture_expr.1 = alloca i64, align 8
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %.omp.stride = alloca i64, align 8
  %.omp.is_last = alloca i32, align 4
  %i = alloca i64, align 8
  store ptr %x, ptr %x.addr, align 4, !tbaa !2
  store i64 %n, ptr %n.addr, align 8, !tbaa !6
  %0 = bitcast ptr %.omp.iv to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr %0) #1
  %1 = bitcast ptr %.capture_expr. to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr %1) #1
  %2 = load i64, ptr %n.addr, align 8, !tbaa !6
  store i64 %2, ptr %.capture_expr., align 8, !tbaa !6
  %3 = bitcast ptr %.capture_expr.1 to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr %3) #1
  %4 = load i64, ptr %.capture_expr., align 8, !tbaa !6
  %sub = sub nsw i64 %4, 0
  %sub2 = sub nsw i64 %sub, 1
  %add = add nsw i64 %sub2, 1
  %div = sdiv i64 %add, 1
  %sub3 = sub nsw i64 %div, 1
  store i64 %sub3, ptr %.capture_expr.1, align 8, !tbaa !6
  %5 = load i64, ptr %.capture_expr., align 8, !tbaa !6
  %cmp = icmp slt i64 0, %5
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %6 = bitcast ptr %.omp.lb to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr %6) #1
  store i64 0, ptr %.omp.lb, align 8, !tbaa !6
  %7 = bitcast ptr %.omp.ub to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr %7) #1
  %8 = load i64, ptr %.capture_expr.1, align 8, !tbaa !6
  store i64 %8, ptr %.omp.ub, align 8, !tbaa !6
  %9 = bitcast ptr %.omp.stride to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr %9) #1
  store i64 1, ptr %.omp.stride, align 8, !tbaa !6
  %10 = bitcast ptr %.omp.is_last to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %10) #1
  store i32 0, ptr %.omp.is_last, align 4, !tbaa !8
  %11 = load i64, ptr %n.addr, align 8, !tbaa !6
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.NUM_THREADS"(i64 %11),
    "QUAL.OMP.SHARED:TYPED"(ptr %x.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %n.addr, i64 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i64 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i64 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i64 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i64 0, i32 1) ]
  %13 = load i64, ptr %.omp.lb, align 8, !tbaa !6
  store i64 %13, ptr %.omp.iv, align 8, !tbaa !6
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %14 = load i64, ptr %.omp.iv, align 8, !tbaa !6
  %15 = load i64, ptr %.omp.ub, align 8, !tbaa !6
  %cmp4 = icmp sle i64 %14, %15
  br i1 %cmp4, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %16 = bitcast ptr %i to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr %16) #1
  %17 = load i64, ptr %.omp.iv, align 8, !tbaa !6
  %mul = mul nsw i64 %17, 1
  %add5 = add nsw i64 0, %mul
  store i64 %add5, ptr %i, align 8, !tbaa !6
  %18 = load ptr, ptr %x.addr, align 4, !tbaa !2
  %19 = load i64, ptr %i, align 8, !tbaa !6
  %idxprom = trunc i64 %19 to i32
  %arrayidx = getelementptr inbounds float, ptr %18, i32 %idxprom
  store float 0.000000e+00, ptr %arrayidx, align 4, !tbaa !10
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %20 = bitcast ptr %i to ptr
  call void @llvm.lifetime.end.p0(i64 8, ptr %20) #1
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %21 = load i64, ptr %.omp.iv, align 8, !tbaa !6
  %add6 = add nsw i64 %21, 1
  store i64 %add6, ptr %.omp.iv, align 8, !tbaa !6
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %22 = bitcast ptr %.omp.is_last to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr %22) #1
  %23 = bitcast ptr %.omp.stride to ptr
  call void @llvm.lifetime.end.p0(i64 8, ptr %23) #1
  %24 = bitcast ptr %.omp.ub to ptr
  call void @llvm.lifetime.end.p0(i64 8, ptr %24) #1
  %25 = bitcast ptr %.omp.lb to ptr
  call void @llvm.lifetime.end.p0(i64 8, ptr %25) #1
  %26 = bitcast ptr %.capture_expr.1 to ptr
  call void @llvm.lifetime.end.p0(i64 8, ptr %26) #1
  %27 = bitcast ptr %.capture_expr. to ptr
  call void @llvm.lifetime.end.p0(i64 8, ptr %27) #1
  %28 = bitcast ptr %.omp.iv to ptr
  call void @llvm.lifetime.end.p0(i64 8, ptr %28) #1
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

attributes #0 = { nounwind uwtable }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nocallback nofree nosync nounwind willreturn }

!llvm.module.flags = !{!0, !1}

!0 = !{i32 1, !"NumRegisterParameters", i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPf", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"long long", !4, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !4, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"float", !4, i64 0}
