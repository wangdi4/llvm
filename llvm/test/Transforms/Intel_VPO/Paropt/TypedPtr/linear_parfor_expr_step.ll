; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s
;
; Test SRC:
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
;

; ModuleID = 'linear_parfor_expr_step.c'
source_filename = "linear_parfor_expr_step.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@yptr = common dso_local global i32* null, align 8
@.str = private unnamed_addr constant [32 x i8] c"y[%d] = %d, yptr = %p, &y = %p\0A\00", align 1
@.str.1 = private unnamed_addr constant [25 x i8] c"y = %d (expected = %ld)\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(i64 %step) #0 {
entry:
  %step.addr = alloca i64, align 8
  %y = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i64 %step, i64* %step.addr, align 8
  store i32 2, i32* %y, align 4
  store i32* %y, i32** @yptr, align 8
  store i32 0, i32* %.omp.lb, align 4
  store i32 4, i32* %.omp.ub, align 4
  %0 = load i64, i64* %step.addr, align 8

; CHECK: [[STEP_VAL:%.+]] = load i64, i64* %step.addr
; CHECK: store i64 [[STEP_VAL]], i64* [[STEP_VAL_CAPTURED:%[a-zA-Z._0-9]+]]
; Check that the captured step value is being sent in through the outlined function.
; CHECK: call void {{.+}} @__kmpc_fork_call(%struct.ident_t* {{.+}}, i32 5, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, i64*, i64*, i32*, i32*, i32*)* @foo.{{.*}} to void (i32*, i32*, ...)*), i64* [[STEP_VAL_CAPTURED]], i64* %step.addr, i32* %.omp.lb, i32* %y, i32* %.omp.ub)

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.LINEAR"(i32* %y, i64 %0), "QUAL.OMP.SHARED"(i64* %step.addr), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"(i32** @yptr) ]
  %2 = load i32, i32* %.omp.lb, align 4
  store i32 %2, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, i32* %.omp.iv, align 4
  %4 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %6 = load i32, i32* %i, align 4
  %7 = load i32, i32* %y, align 4
  %8 = load i32*, i32** @yptr, align 8
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([32 x i8], [32 x i8]* @.str, i64 0, i64 0), i32 %6, i32 %7, i32* %8, i32* %y)
  %9 = load i64, i64* %step.addr, align 8
  %10 = load i32, i32* %y, align 4
  %conv = sext i32 %10 to i64
  %add1 = add nsw i64 %conv, %9
  %conv2 = trunc i64 %add1 to i32
  store i32 %conv2, i32* %y, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %11 = load i32, i32* %.omp.iv, align 4
  %add3 = add nsw i32 %11, 1
  store i32 %add3, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %12 = load i32, i32* %y, align 4
  %13 = load i64, i64* %step.addr, align 8
  %mul4 = mul nsw i64 %13, 5
  %add5 = add nsw i64 2, %mul4
  %call6 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str.1, i64 0, i64 0), i32 %12, i64 %add5)
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #3 {
entry:
  call void @foo(i64 2)
  ret i32 0
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
