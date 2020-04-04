; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S < %s | FileCheck %s
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt'  -S | FileCheck %s
;
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
; ModuleID = 'for_fp_lp.c'
source_filename = "for_fp_lp.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@y = common dso_local global i32 0, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, i32* %.omp.lb, align 4
  store i32 9, i32* %.omp.ub, align 4

; Check that the ident_t flag has bits for OpenMP version 5.0
; CHECK: @{{[^ ]+}} = private unnamed_addr global %__struct.ident_t { i32 0, i32 838860802, {{[^}]+}}

; Initialization of y's local copy
; CHECK: [[LOAD1:%[a-zA-Z._0-9]+]] = load i32, i32* @y
; CHECK: store i32 [[LOAD1]], i32* [[Y_LP:%[a-zA-Z._0-9]+]]
; CHECK: call void @__kmpc_barrier
; CHECK: call void @__kmpc_dispatch_init_4

; Check for the copyout to the original y
; CHECK: [[ADD:%[a-zA-Z._0-9]+]] = add nsw i32 {{[^ ]+}}, 1
; CHECK: [[CMP1:%[a-zA-Z._0-9]+]] = icmp sle i32 [[ADD]], %{{[^ ]+}}
; CHECK: br i1 [[CMP1]], label %{{[^,]+}}, label %[[L:[a-zA-Z._0-9]+]]
; CHECK: [[L]]:
; CHECK-NEXT: [[LOAD2:%[a-zA-Z._0-9]+]] = load i32, i32* %is.last
; CHECK-NEXT: [[CMP2:%[a-zA-Z._0-9]+]] = icmp ne i32 [[LOAD2]], 0
; CHECK-NEXT: br i1 [[CMP2]], label %[[LAST_THEN:[a-zA-Z._0-9]+]], label %{{[^ ,]+}}
; CHECK: [[LAST_THEN]]:
; CHECK-NEXT: [[LOAD3:%[a-zA-Z._0-9]+]] = load i32, i32* [[Y_LP]]
; CHECK-NEXT: store i32 [[LOAD3]], i32* @y

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.LASTPRIVATE"(i32* @y), "QUAL.OMP.FIRSTPRIVATE"(i32* @y), "QUAL.OMP.SCHEDULE.RUNTIME"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
  %1 = load i32, i32* %.omp.lb, align 4
  store i32 %1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, i32* %.omp.iv, align 4
  %3 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  store i32 50, i32* @y, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %5 = load i32, i32* %.omp.iv, align 4
  %add1 = add nsw i32 %5, 1
  store i32 %add1, i32* %.omp.iv, align 4
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

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
