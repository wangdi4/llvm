; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s

; Test src:
;
; #include <stdio.h>
;
; int *yptr;
; #define INIT 2
; #define NUM_ITERS 5
;
; __attribute__((noinline)) void foo(long step) {
;   int y = INIT;
;   yptr = &y;
;
; #pragma omp parallel for linear(y: step)
;   for (int i = 0; i < NUM_ITERS; i++) {
;     printf("y[%d] = %d, yptr = %p, &y = %p\n", i, y, yptr, &y);
;     y+= step;
;   }
;   printf("y = %d (expected = %ld)\n", y, INIT + step*NUM_ITERS);
; }
;
; int main() {
;   foo(2);
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@yptr = dso_local global ptr null, align 8
@.str = private unnamed_addr constant [32 x i8] c"y[%d] = %d, yptr = %p, &y = %p\0A\00", align 1
@.str.1 = private unnamed_addr constant [25 x i8] c"y = %d (expected = %ld)\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(i64 noundef %step) #0 {
entry:
  %step.addr = alloca i64, align 8
  %y = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i64 %step, ptr %step.addr, align 8
  store i32 2, ptr %y, align 4
  store ptr %y, ptr @yptr, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 4, ptr %.omp.ub, align 4
  %0 = load i64, ptr %step.addr, align 8

; CHECK: [[STEP_VAL:%.+]] = load i64, ptr %step.addr
; CHECK: store i64 [[STEP_VAL]], ptr [[STEP_VAL_CAPTURED:%[a-zA-Z._0-9]+]]
; Check that the captured step value is being sent in through the outlined function.
; CHECK: call void {{.+}} @__kmpc_fork_call(ptr {{.+}}, i32 5, ptr @foo.{{.*}}, ptr [[STEP_VAL_CAPTURED]], ptr %step.addr, ptr %.omp.lb, ptr %y, ptr %.omp.ub)

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.LINEAR:TYPED"(ptr %y, i32 0, i32 1, i64 %0),
    "QUAL.OMP.SHARED:TYPED"(ptr @yptr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %step.addr, i64 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]
  %2 = load i32, ptr %.omp.lb, align 4
  store i32 %2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, ptr %.omp.iv, align 4
  %4 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %6 = load i32, ptr %i, align 4
  %7 = load i32, ptr %y, align 4
  %8 = load ptr, ptr @yptr, align 8
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %6, i32 noundef %7, ptr noundef %8, ptr noundef %y) #1
  %9 = load i64, ptr %step.addr, align 8
  %10 = load i32, ptr %y, align 4
  %conv = sext i32 %10 to i64
  %add1 = add nsw i64 %conv, %9
  %conv2 = trunc i64 %add1 to i32
  store i32 %conv2, ptr %y, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %11 = load i32, ptr %.omp.iv, align 4
  %add3 = add nsw i32 %11, 1
  store i32 %add3, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %12 = load i32, ptr %y, align 4
  %13 = load i64, ptr %step.addr, align 8
  %mul4 = mul nsw i64 %13, 5
  %add5 = add nsw i64 2, %mul4
  %call6 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef %12, i64 noundef %add5)
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(ptr noundef, ...) #2

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #3 {
entry:
  call void @foo(i64 noundef 2)
  ret i32 0
}

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
