; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; extern void foo(float);
; void goo() {
;   int n = 100;
;   float a[n];
; #pragma omp distribute parallel for dist_schedule(static, 8) schedule(static, 2)
;   for (int k = 0; k < n; k += 1) {
;     foo(a[k]);
;   }
; }

; CHECK-NOT: %{{[0-9]+}} = call token @llvm.directive.region.entry() {{.*}}
; CHECK-NOT: call void @llvm.directive.region.exit(token %{{[0-9]+}}) {{.*}}
; CHECK: call void {{.*}} @__kmpc_fork_call({{.*}})
; CHECK: call void @__kmpc_team_static_init_4({{.*}})

; CHECK: declare !callback ![[NUM1:[0-9]+]]{{.*}}void @__kmpc_fork_call({{.*}}, ...)
; CHECK: ![[NUM1]] = !{![[NUM2:[0-9]+]]}
; CHECK: ![[NUM2]] = !{i64 2, i64 -1, i64 -1, i1 true}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @goo() #0 {
entry:
  %n = alloca i32, align 4
  %saved_stack = alloca ptr, align 8
  %__vla_expr0 = alloca i64, align 8
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %omp.vla.tmp = alloca i64, align 8
  %k = alloca i32, align 4
  store i32 100, ptr %n, align 4
  %0 = load i32, ptr %n, align 4
  %1 = zext i32 %0 to i64
  %2 = call ptr @llvm.stacksave()
  store ptr %2, ptr %saved_stack, align 8
  %vla = alloca float, i64 %1, align 16
  store i64 %1, ptr %__vla_expr0, align 8
  %3 = load i32, ptr %n, align 4
  store i32 %3, ptr %.capture_expr.0, align 4
  %4 = load i32, ptr %.capture_expr.0, align 4
  %sub = sub nsw i32 %4, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.1, align 4
  %5 = load i32, ptr %.capture_expr.0, align 4
  %cmp = icmp slt i32 0, %5
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, ptr %.omp.lb, align 4
  %6 = load i32, ptr %.capture_expr.1, align 4
  store i32 %6, ptr %.omp.ub, align 4
  store i64 %1, ptr %omp.vla.tmp, align 8
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.DIST_SCHEDULE.STATIC"(i32 8),
    "QUAL.OMP.SCHEDULE.STATIC"(i32 2),
    "QUAL.OMP.SHARED:TYPED"(ptr %vla, float 0.000000e+00, i64 %1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %k, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %omp.vla.tmp, i64 0, i32 1) ]
  %8 = load i64, ptr %omp.vla.tmp, align 8
  %9 = load i32, ptr %.omp.lb, align 4
  store i32 %9, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %10 = load i32, ptr %.omp.iv, align 4
  %11 = load i32, ptr %.omp.ub, align 4
  %cmp3 = icmp sle i32 %10, %11
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %12 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %12, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, ptr %k, align 4
  %13 = load i32, ptr %k, align 4
  %idxprom = sext i32 %13 to i64
  %arrayidx = getelementptr inbounds float, ptr %vla, i64 %idxprom
  %14 = load float, ptr %arrayidx, align 4
  call void @foo(float noundef %14) #2
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %15 = load i32, ptr %.omp.iv, align 4
  %add5 = add nsw i32 %15, 1
  store i32 %add5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %16 = load ptr, ptr %saved_stack, align 8
  call void @llvm.stackrestore(ptr %16)
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare ptr @llvm.stacksave() #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare dso_local void @foo(float noundef) #3

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.stackrestore(ptr) #1

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind }
attributes #3 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
