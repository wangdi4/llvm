; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Check the codegen for GRAINSIZE amd NUM_TASKS w/o STRICT
; The test contains hand-modified IR to add "STRICT" to "QUAL.OMP.GRAINSIZE" and "QUAL.OMP.NUM_TASKS"
;
; Test src:
;
; void foo() {
;  #pragma omp taskloop num_tasks(strict: 10)
;  for (int i = 0; i < 10; ++i);
;
;  #pragma omp taskloop num_tasks(10)
;  for (int i = 0; i < 10; ++i);
;
; #pragma omp taskloop grainsize(strict: 5)
;  for (int i = 0; i < 5; ++i);
;  }
;
; #pragma omp taskloop grainsize(5)
;  for (int i = 0; i < 5; ++i);
;  }

; Check for NumTask with STRICT
; CHECK: call void @__kmpc_taskloop_5(ptr {{.*}}, i32  %{{.*}}, ptr  %{{.*}}, i32 {{.*}}, ptr %{{.*}}, ptr %{{.*}}, i64 %{{.*}}, i32 {{.*}}, i32 2, i64 10, i32 1, ptr {{.*}})

; Check for NumTask without STRICT
; CHECK: call void @__kmpc_taskloop_5(ptr {{.*}}, i32  %{{.*}}, ptr  %{{.*}}, i32 {{.*}}, ptr %{{.*}}, ptr %{{.*}}, i64 %{{.*}}, i32 {{.*}}, i32 2, i64 10, i32 0, ptr {{.*}})

; Check for Grainsize with STRICT
; CHECK: call void @__kmpc_taskloop_5(ptr {{.*}}, i32  %{{.*}}, ptr  %{{.*}}, i32 {{.*}}, ptr %{{.*}}, ptr %{{.*}}, i64 %{{.*}}, i32 {{.*}}, i32 1, i64 5, i32 1, ptr {{.*}})

; Check for Grainsize without STRICT
; CHECK: call void @__kmpc_taskloop_5(ptr {{.*}}, i32  %{{.*}}, ptr  %{{.*}}, i32 {{.*}}, ptr %{{.*}}, ptr %{{.*}}, i64 %{{.*}}, i32 {{.*}}, i32 1, i64 5, i32 0, ptr {{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo() {
entry:
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
  %tmp17 = alloca i32, align 4
  %.omp.iv18 = alloca i32, align 4
  %.omp.lb19 = alloca i32, align 4
  %.omp.ub20 = alloca i32, align 4
  %i24 = alloca i32, align 4
  %tmp32 = alloca i32, align 4
  %.omp.iv33 = alloca i32, align 4
  %.omp.lb34 = alloca i32, align 4
  %.omp.ub35 = alloca i32, align 4
  %i39 = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(),
    "QUAL.OMP.IMPLICIT"() ]

  store i32 0, ptr %.omp.lb, align 4
  store i32 9, ptr %.omp.ub, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(),
    "QUAL.OMP.NUM_TASKS:STRICT"(i32 10),
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
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %6 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %6, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASKLOOP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASKGROUP"() ]
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(),
    "QUAL.OMP.IMPLICIT"() ]

  store i32 0, ptr %.omp.lb4, align 4
  store i32 9, ptr %.omp.ub5, align 4
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(),
    "QUAL.OMP.NUM_TASKS"(i32 10),
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
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TASKLOOP"() ]
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.TASKGROUP"() ]
  %14 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(),
     "QUAL.OMP.IMPLICIT"() ]

  store i32 0, ptr %.omp.lb19, align 4
  store i32 9, ptr %.omp.ub20, align 4
  %15 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(),
     "QUAL.OMP.GRAINSIZE:STRICT"(i32 5),
     "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv18, i32 0),
     "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb19, i32 0, i32 1),
     "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub20, i32 0),
     "QUAL.OMP.PRIVATE:TYPED"(ptr %i24, i32 0, i32 1) ]

  %16 = load i32, ptr %.omp.lb19, align 4
  store i32 %16, ptr %.omp.iv18, align 4
  br label %omp.inner.for.cond21

omp.inner.for.cond21:                             ; preds = %omp.inner.for.inc28, %omp.loop.exit16
  %17 = load i32, ptr %.omp.iv18, align 4
  %18 = load i32, ptr %.omp.ub20, align 4
  %cmp22 = icmp sle i32 %17, %18
  br i1 %cmp22, label %omp.inner.for.body23, label %omp.inner.for.end30

omp.inner.for.body23:                             ; preds = %omp.inner.for.cond21
  %19 = load i32, ptr %.omp.iv18, align 4
  %mul25 = mul nsw i32 %19, 1
  %add26 = add nsw i32 0, %mul25
  store i32 %add26, ptr %i24, align 4
  br label %omp.body.continue27

omp.body.continue27:                              ; preds = %omp.inner.for.body23
  br label %omp.inner.for.inc28

omp.inner.for.inc28:                              ; preds = %omp.body.continue27
  %20 = load i32, ptr %.omp.iv18, align 4
  %add29 = add nsw i32 %20, 1
  store i32 %add29, ptr %.omp.iv18, align 4
  br label %omp.inner.for.cond21

omp.inner.for.end30:                              ; preds = %omp.inner.for.cond21
  br label %omp.loop.exit31

omp.loop.exit31:                                  ; preds = %omp.inner.for.end30
  call void @llvm.directive.region.exit(token %15) [ "DIR.OMP.END.TASKLOOP"() ]
  call void @llvm.directive.region.exit(token %14) [ "DIR.OMP.END.TASKGROUP"() ]
  %21 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(),
     "QUAL.OMP.IMPLICIT"() ]

  store i32 0, ptr %.omp.lb34, align 4
  store i32 9, ptr %.omp.ub35, align 4
  %22 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(),
     "QUAL.OMP.GRAINSIZE"(i32 5),
     "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv33, i32 0),
     "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb34, i32 0, i32 1),
     "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub35, i32 0),
     "QUAL.OMP.PRIVATE:TYPED"(ptr %i39, i32 0, i32 1) ]

  %23 = load i32, ptr %.omp.lb34, align 4
  store i32 %23, ptr %.omp.iv33, align 4
  br label %omp.inner.for.cond36

omp.inner.for.cond36:                             ; preds = %omp.inner.for.inc43, %omp.loop.exit31
  %24 = load i32, ptr %.omp.iv33, align 4
  %25 = load i32, ptr %.omp.ub35, align 4
  %cmp37 = icmp sle i32 %24, %25
  br i1 %cmp37, label %omp.inner.for.body38, label %omp.inner.for.end45

omp.inner.for.body38:                             ; preds = %omp.inner.for.cond36
  %26 = load i32, ptr %.omp.iv33, align 4
  %mul40 = mul nsw i32 %26, 1
  %add41 = add nsw i32 0, %mul40
  store i32 %add41, ptr %i39, align 4
  br label %omp.body.continue42

omp.body.continue42:                              ; preds = %omp.inner.for.body38
  br label %omp.inner.for.inc43

omp.inner.for.inc43:                              ; preds = %omp.body.continue42
  %27 = load i32, ptr %.omp.iv33, align 4
  %add44 = add nsw i32 %27, 1
  store i32 %add44, ptr %.omp.iv33, align 4
  br label %omp.inner.for.cond36

omp.inner.for.end45:                              ; preds = %omp.inner.for.cond36
  br label %omp.loop.exit46

omp.loop.exit46:                                  ; preds = %omp.inner.for.end45
  call void @llvm.directive.region.exit(token %22) [ "DIR.OMP.END.TASKLOOP"() ]
  call void @llvm.directive.region.exit(token %21) [ "DIR.OMP.END.TASKGROUP"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
