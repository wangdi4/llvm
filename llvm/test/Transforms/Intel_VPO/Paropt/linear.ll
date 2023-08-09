; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src (only foo function is used in this lit test):
;
; #include <stdio.h>
;
; short y = 0;
;
; void foo() {
; #pragma omp parallel for linear(y : 2)
;   for (long i = 0; i < 10; i++) {
;     printf("y[%ld] = %d\n", i, y);
;     y += 2;
;   }
; }
;
; int main() {
;
;   foo();
;
;   printf("Final: y = %d\n", y);
;   if (y == 20) {
;       printf("PASSED\n");
;     return 0;
;   }
;
;   printf("FAILED\n");
;   return 1;
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@y = dso_local global i16 0, align 2
@.str = private unnamed_addr constant [13 x i8] c"y[%ld] = %d\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %tmp = alloca i64, align 8
  %.omp.iv = alloca i64, align 8
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %i = alloca i64, align 8
  store i64 0, ptr %.omp.lb, align 8
  store i64 9, ptr %.omp.ub, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.LINEAR:TYPED"(ptr @y, i16 0, i32 1, i32 2),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i64 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i64 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i64 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i64 0, i32 1) ]

; Initial copy of linear var
; CHECK: [[LOAD1:%[a-zA-Z._0-9]+]] = load i16, ptr @y
; CHECK: store i16 [[LOAD1]], ptr [[LINEAR_INIT:%[a-zA-Z._0-9]+]]
; CHECK: call void @__kmpc_barrier{{.*}}
; CHECK: call void @__kmpc_for_static_init_8(ptr @{{[^, ]+}}, i32 %{{[^, ]+}}, i32 34, ptr %{{[^, ]+}}, ptr [[CHUNK_LB_PTR:%[^, ]+]]{{.*}}

; Initialization of linear var per chunk
; CHECK: [[CHUNK_LB:%[^ ]+]] = load i64, ptr [[CHUNK_LB_PTR]]
; CHECK: [[LOAD2:%[a-zA-Z._0-9]+]] = load i16, ptr [[LINEAR_INIT]]
; CHECK: [[MUL:%[a-zA-Z._0-9]+]] = mul i64 [[CHUNK_LB]], 2
; CHECK: [[CAST1:%[a-zA-Z._0-9]+]] = sext i16 [[LOAD2]] to i64
; CHECK: [[ADD:%[a-zA-Z._0-9]+]] = add i64 [[CAST1]], [[MUL]]
; CHECK: [[CAST2:%[a-zA-Z._0-9]+]] = trunc i64 [[ADD]] to i16
; CHECK: store i16 [[CAST2]], ptr [[LINEAR_LOCAL:%[a-zA-Z._0-9]+]]

; Final copyout for linear var
; CHECK-DAG: store i16 [[LOAD3:%[a-zA-Z._0-9]+]], ptr @y
; CHECK-DAG: [[LOAD3]] = load i16, ptr [[LINEAR_LOCAL]]

  %1 = load i64, ptr %.omp.lb, align 8
  store i64 %1, ptr %.omp.iv, align 8
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i64, ptr %.omp.iv, align 8
  %3 = load i64, ptr %.omp.ub, align 8
  %cmp = icmp sle i64 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i64, ptr %.omp.iv, align 8
  %mul = mul nsw i64 %4, 1
  %add = add nsw i64 0, %mul
  store i64 %add, ptr %i, align 8
  %5 = load i64, ptr %i, align 8
  %6 = load i16, ptr @y, align 2
  %conv = sext i16 %6 to i32
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i64 noundef %5, i32 noundef %conv) #1
  %7 = load i16, ptr @y, align 2
  %conv1 = sext i16 %7 to i32
  %add2 = add nsw i32 %conv1, 2
  %conv3 = trunc i32 %add2 to i16
  store i16 %conv3, ptr @y, align 2
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i64, ptr %.omp.iv, align 8
  %add4 = add nsw i64 %8, 1
  store i64 %add4, ptr %.omp.iv, align 8
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
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
