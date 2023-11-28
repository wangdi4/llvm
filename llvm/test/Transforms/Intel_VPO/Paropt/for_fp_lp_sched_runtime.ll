; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; int y;
;
; void foo() {
; #pragma omp for lastprivate(y) firstprivate(y) schedule(runtime)
;   for (int i = 0; i < 10; i++) {
;     y = 50;
;   }
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@y = dso_local global i32 0, align 4

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

; Check that the ident_t flag has bits for OpenMP version 5.0
; CHECK: @{{[^ ]+}} = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, {{[^}]+}}

; Initialization of y's local copy
; CHECK: [[LOAD1:%[a-zA-Z._0-9]+]] = load i32, ptr @y
; CHECK: store i32 [[LOAD1]], ptr [[Y_LP:%[a-zA-Z._0-9]+]]
; CHECK: call void @__kmpc_barrier
; CHECK: call void @__kmpc_dispatch_init_4

; Check for the copyout to the original y
; CHECK: [[ADD:%[a-zA-Z._0-9]+]] = add nsw i32 {{[^ ]+}}, 1
; CHECK: [[CMP1:%[a-zA-Z._0-9]+]] = icmp sle i32 [[ADD]], %{{[^ ]+}}
; CHECK: br i1 [[CMP1]], label %{{[^,]+}}, label %[[L:[a-zA-Z._0-9]+]]
; CHECK: [[L]]:
; CHECK-NEXT: [[LOAD2:%[a-zA-Z._0-9]+]] = load i32, ptr %is.last
; CHECK-NEXT: [[CMP2:%[a-zA-Z._0-9]+]] = icmp ne i32 [[LOAD2]], 0
; CHECK-NEXT: br i1 [[CMP2]], label %[[LAST_THEN:[a-zA-Z._0-9]+]], label %{{[^ ,]+}}
; CHECK: [[LAST_THEN]]:
; CHECK-NEXT: [[LOAD3:%[a-zA-Z._0-9]+]] = load i32, ptr [[Y_LP]]
; CHECK-NEXT: store i32 [[LOAD3]], ptr @y

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.LASTPRIVATE:TYPED"(ptr @y, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @y, i32 0, i32 1),
    "QUAL.OMP.SCHEDULE.RUNTIME"(i32 0),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]
  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  store i32 50, ptr @y, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %5 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %5, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
