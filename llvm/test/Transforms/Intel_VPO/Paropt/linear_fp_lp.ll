; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s

; Test src (only foo function is used in this lit test):
;
; #include <stdio.h>
;
; int step = -1;
; short y = 0;
; int x = 0;
;
; void foo() {
; #pragma omp parallel for linear(y : step) firstprivate(x) lastprivate(x)
;   for (unsigned int i = 0; i < 10; i++) {
;     printf("y[%d] = %d\n", i, y);
;     y += 100;
;     x = i;
;   }
; }
;
; int main() {
;
;   foo();
;
;   printf("Final: x = %d, y = %d\n", x, y);
;   if (x == 9 && y == 91) {
;       printf("PASSED\n");
;     return 0;
;   }
;
;   printf("FAILED\n");
;   return 1;
; }

; Note: The test has a different step in the body of the loop than
; the one specified on the clause.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@step = dso_local global i32 -1, align 4
@y = dso_local global i16 0, align 2
@x = dso_local global i32 0, align 4
@.str = private unnamed_addr constant [12 x i8] c"y[%d] = %d\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, ptr %.omp.lb, align 4
  store i32 9, ptr %.omp.ub, align 4
  %0 = load i32, ptr @step, align 4

; CHECK: [[STEP_VAL:%[a-zA-Z._0-9]+]] = load i32, ptr @step, align 4
; Check that the value of step is stored to a pointer, and then sent in
; though the entry.
; CHECK: store i32 [[STEP_VAL]], ptr [[STEP_VAL_CAPTURED:%[a-zA-Z._0-9]+]]
; CHECK: void @foo{{[a-zA-Z._0-9]*}}(ptr %{{.*}}, ptr %{{.*}}, ptr [[STEP_VAL_CAPTURED]], ptr %.omp.lb, ptr %.omp.ub)

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.LINEAR:TYPED"(ptr @y, i16 0, i32 1, i32 %0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @x, i32 0, i32 1),
    "QUAL.OMP.LASTPRIVATE:TYPED"(ptr @x, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr @step, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

; Initial copy of linear var
; CHECK: [[STEP_VAL_INREGION:%[a-zA-Z._0-9]+]] = load i32, ptr [[STEP_VAL_CAPTURED]]
; CHECK: [[LOAD1:%[a-zA-Z._0-9]+]] = load i16, ptr @y
; CHECK: store i16 [[LOAD1]], ptr [[LINEAR_INIT:%[a-zA-Z._0-9]+]]
; CHECK: call void @__kmpc_barrier{{.*}}
; CHECK: call void @__kmpc_for_static_init_4u(ptr @{{[^, ]+}}, i32 %{{[^, ]+}}, i32 34, ptr %{{[^, ]+}}, ptr [[CHUNK_LB_PTR:%[^, ]+]]{{.*}}

; Initialization of linear var per chunk
; CHECK: [[CHUNK_LB:%[^ ]+]] = load i32, ptr [[CHUNK_LB_PTR]]
; CHECK: [[LOAD2:%[a-zA-Z._0-9]+]] = load i16, ptr [[LINEAR_INIT]]
; CHECK: [[MUL:%[a-zA-Z._0-9]+]] = mul i32 [[CHUNK_LB]], [[STEP_VAL_INREGION]]
; CHECK: [[CAST1:%[a-zA-Z._0-9]+]] = sext i16 [[LOAD2]] to i32
; CHECK: [[ADD:%[a-zA-Z._0-9]+]] = add i32 [[CAST1]], [[MUL]]
; CHECK: [[CAST2:%[a-zA-Z._0-9]+]] = trunc i32 [[ADD]] to i16
; CHECK: store i16 [[CAST2]], ptr [[LINEAR_LOCAL:%[a-zA-Z._0-9]+]]

; Make sure that only one barrier is emitted even though the test has
; fp+lp as well as linear vars.
; CHECK-NOT: call void @__kmpc_barrier{{.*}}

; Final copyout for linear var
; CHECK-DAG: store i16 [[LOAD3:%[a-zA-Z._0-9]+]], ptr @y
; CHECK-DAG: [[LOAD3]] = load i16, ptr [[LINEAR_LOCAL]]

  %2 = load i32, ptr %.omp.lb, align 4
  store i32 %2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, ptr %.omp.iv, align 4
  %4 = load i32, ptr %.omp.ub, align 4
  %add = add i32 %4, 1
  %cmp = icmp ult i32 %3, %add
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, ptr %.omp.iv, align 4
  %mul = mul i32 %5, 1
  %add1 = add i32 0, %mul
  store i32 %add1, ptr %i, align 4
  %6 = load i32, ptr %i, align 4
  %7 = load i16, ptr @y, align 2
  %conv = sext i16 %7 to i32
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %6, i32 noundef %conv) #1
  %8 = load i16, ptr @y, align 2
  %conv2 = sext i16 %8 to i32
  %add3 = add nsw i32 %conv2, 100
  %conv4 = trunc i32 %add3 to i16
  store i16 %conv4, ptr @y, align 2
  %9 = load i32, ptr %i, align 4
  store i32 %9, ptr @x, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, ptr %.omp.iv, align 4
  %add5 = add nuw i32 %10, 1
  store i32 %add5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(ptr noundef, ...) #2

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
