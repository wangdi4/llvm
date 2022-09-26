; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Test src:
;
; void foo(int n) {
;   int a[n];
;   int i, res = 0;
; #pragma omp task private(a)
;   a[0] = 3;
; #pragma omp task firstprivate(a)
;   a[0] = 3;
; #pragma omp task shared(a)
;   a[0] = 3;
; #pragma omp taskloop lastprivate(a) nogroup
;   for (i = 0; i < n; ++i)
;     a[i] = 0;
; #pragma omp taskgroup task_reduction(+ : res)
;   {
; #pragma omp task firstprivate(a) in_reduction(+ : res)
;     res = res + a[0];
; #pragma omp taskloop firstprivate(a) in_reduction(+ : res)
;     for (i = 1; i < n; ++i)
;       res += a[i];
;   }
; }

; Check that paropt transform pass does not pass typed VLA size as the argument
; of task/taskloop's outlined function.

; CHECK-COUNT-6: %taskt.withprivates{{.*}} = alloca %__struct.kmp_task_t_with_privates{{.*}}, align 8
; CHECK-COUNT-6: %.task.alloc{{.*}} = call i8* @__kmpc_omp_task_alloc{{.*}}@foo.DIR.OMP.TASK{{.*}}
; CHECK-CHECK-6: define internal void @{{.*}}DIR.OMP.TASK{{.*}}(i32 %tid, ptr %taskt.withprivates{{[^ ,]}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(i32 noundef %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  %saved_stack = alloca i8*, align 8
  %__vla_expr0 = alloca i64, align 8
  %i = alloca i32, align 4
  %res = alloca i32, align 4
  %omp.vla.tmp = alloca i64, align 8
  %omp.vla.tmp1 = alloca i64, align 8
  %omp.vla.tmp3 = alloca i64, align 8
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %omp.vla.tmp7 = alloca i64, align 8
  %omp.vla.tmp12 = alloca i64, align 8
  %omp.vla.tmp13 = alloca i64, align 8
  %omp.vla.tmp16 = alloca i64, align 8
  %tmp17 = alloca i32, align 4
  %.capture_expr.2 = alloca i32, align 4
  %.capture_expr.3 = alloca i32, align 4
  %.omp.iv23 = alloca i32, align 4
  %.omp.lb24 = alloca i32, align 4
  %.omp.ub25 = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4
  %0 = load i32, i32* %n.addr, align 4
  %1 = zext i32 %0 to i64
  %2 = call i8* @llvm.stacksave()
  store i8* %2, i8** %saved_stack, align 8
  %vla = alloca i32, i64 %1, align 16
  store i64 %1, i64* %__vla_expr0, align 8
  store i32 0, i32* %res, align 4
  store i64 %1, i64* %omp.vla.tmp, align 8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.PRIVATE:TYPED"(i32* %vla, i32 0, i64 %1),
    "QUAL.OMP.PRIVATE:TYPED"(i64* %omp.vla.tmp, i64 0, i32 1) ]

  %4 = load i64, i64* %omp.vla.tmp, align 8
  %arrayidx = getelementptr inbounds i32, i32* %vla, i64 0
  store i32 3, i32* %arrayidx, align 16
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TASK"() ]

  store i64 %1, i64* %omp.vla.tmp1, align 8
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* %vla, i32 0, i64 %1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i64* %omp.vla.tmp1, i64 0, i32 1) ]

  %6 = load i64, i64* %omp.vla.tmp1, align 8
  %arrayidx2 = getelementptr inbounds i32, i32* %vla, i64 0
  store i32 3, i32* %arrayidx2, align 16
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TASK"() ]

  store i64 %1, i64* %omp.vla.tmp3, align 8
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.SHARED:TYPED"(i32* %vla, i32 0, i64 %1),
    "QUAL.OMP.SHARED:TYPED"(i64* %omp.vla.tmp3, i64 0, i32 1) ]

  %8 = load i64, i64* %omp.vla.tmp3, align 8
  %arrayidx4 = getelementptr inbounds i32, i32* %vla, i64 0
  store i32 3, i32* %arrayidx4, align 16
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.TASK"() ]

  %9 = load i32, i32* %n.addr, align 4
  store i32 %9, i32* %.capture_expr.0, align 4
  %10 = load i32, i32* %.capture_expr.0, align 4
  %sub = sub nsw i32 %10, 0
  %sub5 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub5, 1
  %div = sdiv i32 %add, 1
  %sub6 = sub nsw i32 %div, 1
  store i32 %sub6, i32* %.capture_expr.1, align 4
  %11 = load i32, i32* %.capture_expr.0, align 4
  %cmp = icmp slt i32 0, %11
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, i32* %.omp.lb, align 4
  %12 = load i32, i32* %.capture_expr.1, align 4
  store i32 %12, i32* %.omp.ub, align 4
  store i64 %1, i64* %omp.vla.tmp7, align 8
  %13 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(),
    "QUAL.OMP.LASTPRIVATE:TYPED"(i32* %vla, i32 0, i64 %1),
    "QUAL.OMP.NOGROUP"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* %n.addr, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32* %i, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(i32* %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(i32* %.omp.ub, i32 0),
    "QUAL.OMP.SHARED:TYPED"(i64* %omp.vla.tmp7, i64 0, i32 1) ]

  %14 = load i64, i64* %omp.vla.tmp7, align 8
  %15 = load i32, i32* %.omp.lb, align 4
  store i32 %15, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %16 = load i32, i32* %.omp.iv, align 4
  %17 = load i32, i32* %.omp.ub, align 4
  %cmp8 = icmp sle i32 %16, %17
  br i1 %cmp8, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %18 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %18, 1
  %add9 = add nsw i32 0, %mul
  store i32 %add9, i32* %i, align 4
  %19 = load i32, i32* %i, align 4
  %idxprom = sext i32 %19 to i64
  %arrayidx10 = getelementptr inbounds i32, i32* %vla, i64 %idxprom
  store i32 0, i32* %arrayidx10, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %20 = load i32, i32* %.omp.iv, align 4
  %add11 = add nsw i32 %20, 1
  store i32 %add11, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %13) [ "DIR.OMP.END.TASKLOOP"() ]

  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  store i64 %1, i64* %omp.vla.tmp12, align 8
  %21 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(i32* %res, i32 0, i32 1) ]

  %22 = load i64, i64* %omp.vla.tmp12, align 8
  store i64 %22, i64* %omp.vla.tmp13, align 8
  %23 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.INREDUCTION.ADD:TYPED"(i32* %res, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* %vla, i32 0, i64 %22),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i64* %omp.vla.tmp13, i64 0, i32 1) ]

  %24 = load i64, i64* %omp.vla.tmp13, align 8
  %25 = load i32, i32* %res, align 4
  %arrayidx14 = getelementptr inbounds i32, i32* %vla, i64 0
  %26 = load i32, i32* %arrayidx14, align 16
  %add15 = add nsw i32 %25, %26
  store i32 %add15, i32* %res, align 4
  call void @llvm.directive.region.exit(token %23) [ "DIR.OMP.END.TASK"() ]

  store i64 %22, i64* %omp.vla.tmp16, align 8
  %27 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(),
    "QUAL.OMP.IMPLICIT"() ]

  %28 = load i32, i32* %n.addr, align 4
  store i32 %28, i32* %.capture_expr.2, align 4
  %29 = load i32, i32* %.capture_expr.2, align 4
  %sub18 = sub nsw i32 %29, 1
  %div19 = sdiv i32 %sub18, 1
  %sub20 = sub nsw i32 %div19, 1
  store i32 %sub20, i32* %.capture_expr.3, align 4
  %30 = load i32, i32* %.capture_expr.2, align 4
  %cmp21 = icmp slt i32 1, %30
  br i1 %cmp21, label %omp.precond.then22, label %omp.precond.end39

omp.precond.then22:                               ; preds = %omp.precond.end
  store i32 0, i32* %.omp.lb24, align 4
  %31 = load i32, i32* %.capture_expr.3, align 4
  store i32 %31, i32* %.omp.ub25, align 4
  %32 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(),
    "QUAL.OMP.INREDUCTION.ADD:TYPED"(i32* %res, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* %n.addr, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* %vla, i32 0, i64 %22),
    "QUAL.OMP.PRIVATE:TYPED"(i32* %i, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(i32* %.omp.iv23, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* %.omp.lb24, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(i32* %.omp.ub25, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i64* %omp.vla.tmp16, i64 0, i32 1) ]

  %33 = load i64, i64* %omp.vla.tmp16, align 8
  %34 = load i32, i32* %.omp.lb24, align 4
  store i32 %34, i32* %.omp.iv23, align 4
  br label %omp.inner.for.cond26

omp.inner.for.cond26:                             ; preds = %omp.inner.for.inc35, %omp.precond.then22
  %35 = load i32, i32* %.omp.iv23, align 4
  %36 = load i32, i32* %.omp.ub25, align 4
  %cmp27 = icmp sle i32 %35, %36
  br i1 %cmp27, label %omp.inner.for.body28, label %omp.inner.for.end37

omp.inner.for.body28:                             ; preds = %omp.inner.for.cond26
  %37 = load i32, i32* %.omp.iv23, align 4
  %mul29 = mul nsw i32 %37, 1
  %add30 = add nsw i32 1, %mul29
  store i32 %add30, i32* %i, align 4
  %38 = load i32, i32* %i, align 4
  %idxprom31 = sext i32 %38 to i64
  %arrayidx32 = getelementptr inbounds i32, i32* %vla, i64 %idxprom31
  %39 = load i32, i32* %arrayidx32, align 4
  %40 = load i32, i32* %res, align 4
  %add33 = add nsw i32 %40, %39
  store i32 %add33, i32* %res, align 4
  br label %omp.body.continue34

omp.body.continue34:                              ; preds = %omp.inner.for.body28
  br label %omp.inner.for.inc35

omp.inner.for.inc35:                              ; preds = %omp.body.continue34
  %41 = load i32, i32* %.omp.iv23, align 4
  %add36 = add nsw i32 %41, 1
  store i32 %add36, i32* %.omp.iv23, align 4
  br label %omp.inner.for.cond26

omp.inner.for.end37:                              ; preds = %omp.inner.for.cond26
  br label %omp.loop.exit38

omp.loop.exit38:                                  ; preds = %omp.inner.for.end37
  call void @llvm.directive.region.exit(token %32) [ "DIR.OMP.END.TASKLOOP"() ]

  br label %omp.precond.end39

omp.precond.end39:                                ; preds = %omp.loop.exit38, %omp.precond.end
  call void @llvm.directive.region.exit(token %27) [ "DIR.OMP.END.TASKGROUP"() ]

  call void @llvm.directive.region.exit(token %21) [ "DIR.OMP.END.TASKGROUP"() ]

  %42 = load i8*, i8** %saved_stack, align 8
  call void @llvm.stackrestore(i8* %42)
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare i8* @llvm.stacksave() #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.stackrestore(i8*) #1

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
