; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S <%s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S <%s | FileCheck %s

; Test checks that we generate the correct version of __kmpc_push_proc_bind based on the modifier used.
; Source IR was hand modified because front end does not generate QUAL.OMP.PROC_BIND.PRIMARY yet

; Test src:
;
; #include <omp.h>
; int main() {
; #pragma omp parallel proc_bind(master)
;   {}
; #pragma omp parallel proc_bind(primary)
;   {}
; #pragma omp parallel for proc_bind(close)
;   for (int i = 0; i < 1000; ++i)
;     ;
; #pragma omp distribute parallel for proc_bind(spread)
;   for (int i = 0; i < 1000; ++i)
;     ;
; #pragma omp parallel
;   {}
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @main() {
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
  store i32 0, ptr %retval, align 4

; CHECK: call void @__kmpc_push_proc_bind(ptr @{{.*}}, i32 %{{.*}}, i32 2)

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
     "QUAL.OMP.PROC_BIND.MASTER"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

; CHECK: call void @__kmpc_push_proc_bind(ptr @{{.*}}, i32 %{{.*}}, i32 2)

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
     "QUAL.OMP.PROC_BIND.PRIMARY"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  store i32 0, ptr %.omp.lb, align 4
  store i32 999, ptr %.omp.ub, align 4

; CHECK: call void @__kmpc_push_proc_bind(ptr @{{.*}}, i32 %{{.*}}, i32 3)

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.PROC_BIND.CLOSE"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  %3 = load i32, ptr %.omp.lb, align 4
  store i32 %3, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, ptr %.omp.iv, align 4
  %5 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %7, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  store i32 0, ptr %.omp.lb4, align 4
  store i32 999, ptr %.omp.ub5, align 4

; CHECK: call void @__kmpc_push_proc_bind(ptr @{{.*}}, i32 %{{.*}}, i32 4)

  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.PROC_BIND.SPREAD"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv3, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb4, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub5, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i9, i32 0, i32 1) ]

  %9 = load i32, ptr %.omp.lb4, align 4
  store i32 %9, ptr %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.cond6:                              ; preds = %omp.inner.for.inc13, %omp.loop.exit
  %10 = load i32, ptr %.omp.iv3, align 4
  %11 = load i32, ptr %.omp.ub5, align 4
  %cmp7 = icmp sle i32 %10, %11
  br i1 %cmp7, label %omp.inner.for.body8, label %omp.inner.for.end15

omp.inner.for.body8:                              ; preds = %omp.inner.for.cond6
  %12 = load i32, ptr %.omp.iv3, align 4
  %mul10 = mul nsw i32 %12, 1
  %add11 = add nsw i32 0, %mul10
  store i32 %add11, ptr %i9, align 4
  br label %omp.body.continue12

omp.body.continue12:                              ; preds = %omp.inner.for.body8
  br label %omp.inner.for.inc13

omp.inner.for.inc13:                              ; preds = %omp.body.continue12
  %13 = load i32, ptr %.omp.iv3, align 4
  %add14 = add nsw i32 %13, 1
  store i32 %add14, ptr %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.end15:                              ; preds = %omp.inner.for.cond6
  br label %omp.loop.exit16

omp.loop.exit16:                                  ; preds = %omp.inner.for.end15
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

; CHECK-NOT: call void @__kmpc_push_proc_bind(ptr @{{.*}}, i32 %{{.*}}, i32 0)

  %14 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]

  call void @llvm.directive.region.exit(token %14) [ "DIR.OMP.END.PARALLEL"() ]
  %15 = load i32, ptr %retval, align 4
  ret i32 %15
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
