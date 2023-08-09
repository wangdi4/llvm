; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src (only foo function is used in this lit test):
;
; #include <stdio.h>
;
; int x[10];
; int *xptr = &x[0];
; long step = 1;
;
; void foo() {
; #pragma omp for linear(xptr : step)
;   for (short i = 0; i < 10; i++) {
;     printf("xptr[%d] = %p\n", i, xptr);
;     xptr++;
;   }
; }
;
; int main() {
;
;   printf("Initial = %p\n", xptr);
;   foo();
;   printf("Final = %p\n", xptr);
;   if (xptr == &x[10]) {
;     printf("PASSED\n");
;     return 0;
;   }
;
;   printf("FAILED\n");
;   return 1;
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x = dso_local global [10 x i32] zeroinitializer, align 16
@xptr = dso_local global ptr @x, align 8
@step = dso_local global i64 1, align 8
@.str = private unnamed_addr constant [15 x i8] c"xptr[%d] = %p\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %tmp = alloca i16, align 2
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i16, align 2
  store i32 0, ptr %.omp.lb, align 4
  store i32 9, ptr %.omp.ub, align 4
  %0 = load i64, ptr @step, align 8
; CHECK: [[LOAD_STEP:%[a-zA-Z._0-9]+]] = load i64, ptr @step, align 8

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.LINEAR:PTR_TO_PTR.TYPED"(ptr @xptr, i32 0, i32 1, i64 %0),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i16 0, i32 1) ]

; Initial copy of linear var
; CHECK: [[LOAD1:%[a-zA-Z._0-9]+]] = load ptr, ptr @xptr
; CHECK: store ptr [[LOAD1]], ptr [[LINEAR_INIT:%[a-zA-Z._0-9]+]]
; CHECK: call void @__kmpc_barrier{{.*}}
; CHECK: call void @__kmpc_for_static_init_4(ptr @{{[^, ]+}}, i32 %{{[^, ]+}}, i32 34, ptr %{{[^, ]+}}, ptr [[CHUNK_LB_PTR:%[^, ]+]]{{.*}}

; Initialization of linear var per chunk
; CHECK: [[CHUNK_LB:%[^ ]+]] = load i32, ptr [[CHUNK_LB_PTR]]
; CHECK: [[LOAD2:%[a-zA-Z._0-9]+]] = load ptr, ptr [[LINEAR_INIT]]
; CHECK: [[CAST1:%[a-zA-Z._0-9]+]] = sext i32 [[CHUNK_LB]] to i64
; CHECK: [[MUL:%[a-zA-Z._0-9]+]] = mul i64 [[CAST1]], [[LOAD_STEP]]
; CHECK: [[GEP:%[a-zA-Z._0-9]+]] = getelementptr inbounds i32, ptr [[LOAD2]], i64 [[MUL]]
; CHECK: store ptr [[GEP]], ptr [[LINEAR_LOCAL:%[a-zA-Z._0-9]+]]

; Final copyout for linear var
; CHECK-DAG: store ptr [[LOAD3:%[a-zA-Z._0-9]+]], ptr @xptr
; CHECK-DAG: [[LOAD3]] = load ptr, ptr [[LINEAR_LOCAL]]

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
  %conv = trunc i32 %add to i16
  store i16 %conv, ptr %i, align 2
  %6 = load i16, ptr %i, align 2
  %conv1 = sext i16 %6 to i32
  %7 = load ptr, ptr @xptr, align 8
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %conv1, ptr noundef %7) #1
  %8 = load ptr, ptr @xptr, align 8
  %incdec.ptr = getelementptr inbounds i32, ptr %8, i32 1
  store ptr %incdec.ptr, ptr @xptr, align 8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, ptr %.omp.iv, align 4
  %add2 = add nsw i32 %9, 1
  store i32 %add2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.LOOP"() ]
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
