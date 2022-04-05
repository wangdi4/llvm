; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S <%s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S <%s | FileCheck %s

; SRC:
; #include <omp.h>
;  int main() {
; #pragma omp parallel proc_bind(master)
;   {}
; #pragma omp parallel for proc_bind(close)
;   for (int i = 0; i < 1000; ++i);
; #pragma omp distribute parallel for proc_bind(spread)
;   for (int i = 0; i < 1000; ++i);
; #pragma omp parallel
;   {}
; }

source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp2 = alloca i32, align 4
  %.omp.iv3 = alloca i32, align 4
  %.omp.lb4 = alloca i32, align 4
  %.omp.ub5 = alloca i32, align 4
  %i9 = alloca i32, align 4
  store i32 0, i32* %retval, align 4

; CHECK: call void @__kmpc_push_proc_kind(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 2)

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PROC_BIND.MASTER"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  store i32 0, i32* %.omp.lb, align 4
  store i32 999, i32* %.omp.ub, align 4

; CHECK: call void @__kmpc_push_proc_kind(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 3)

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.PROC_BIND.CLOSE"(), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
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
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %6 = load i32, i32* %.omp.iv, align 4
  %add1 = add nsw i32 %6, 1
  store i32 %add1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  store i32 0, i32* %.omp.lb4, align 4
  store i32 999, i32* %.omp.ub5, align 4

; CHECK: call void @__kmpc_push_proc_kind(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 4)

  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.PROC_BIND.SPREAD"(), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv3), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb4), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub5), "QUAL.OMP.PRIVATE"(i32* %i9) ]
  %8 = load i32, i32* %.omp.lb4, align 4
  store i32 %8, i32* %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.cond6:                              ; preds = %omp.inner.for.inc13, %omp.loop.exit
  %9 = load i32, i32* %.omp.iv3, align 4
  %10 = load i32, i32* %.omp.ub5, align 4
  %cmp7 = icmp sle i32 %9, %10
  br i1 %cmp7, label %omp.inner.for.body8, label %omp.inner.for.end15

omp.inner.for.body8:                              ; preds = %omp.inner.for.cond6
  %11 = load i32, i32* %.omp.iv3, align 4
  %mul10 = mul nsw i32 %11, 1
  %add11 = add nsw i32 0, %mul10
  store i32 %add11, i32* %i9, align 4
  br label %omp.body.continue12

omp.body.continue12:                              ; preds = %omp.inner.for.body8
  br label %omp.inner.for.inc13

omp.inner.for.inc13:                              ; preds = %omp.body.continue12
  %12 = load i32, i32* %.omp.iv3, align 4
  %add14 = add nsw i32 %12, 1
  store i32 %add14, i32* %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.end15:                              ; preds = %omp.inner.for.cond6
  br label %omp.loop.exit16

omp.loop.exit16:                                  ; preds = %omp.inner.for.end15
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

; CHECK-NOT: call void @__kmpc_push_proc_kind(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 0)

  %13 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %13) [ "DIR.OMP.END.PARALLEL"() ]
  %14 = load i32, i32* %retval, align 4
  ret i32 %14
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

