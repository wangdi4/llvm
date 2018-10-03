; RUN: opt < %s -domtree -loops -lcssa-verification -vpo-wrncollection -vpo-wrninfo -loops -vpo-paropt -S | FileCheck --check-prefixes=CHECK-COMMON,CHECK-DEFAULT %s
; RUN: opt < %s -domtree -loops -lcssa-verification -vpo-wrncollection -vpo-wrninfo -loops -vpo-paropt -csa-omp-loop-workers-default=2 -S | FileCheck --check-prefixes=CHECK-COMMON,CHECK-WORKERS %s
; RUN: opt < %s -domtree -loops -lcssa-verification -vpo-wrncollection -vpo-wrninfo -loops -vpo-paropt -csa-omp-loop-workers-default=3 -csa-omp-loop-spmd-mode-default=4 -S | FileCheck --check-prefixes=CHECK-COMMON,CHECK-MODE %s
; REQUIRES: csa-registered-target
;
; Test internal options which specify default number of workers and default
; spmdization mode for OpenMP loops with no num_threads and/or no schedule
; clauses.

target triple = "csa"

; CHECK-LABEL: @foo.no.num_threads.no.schedule
define void @foo.no.num_threads.no.schedule() {
entry:
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %0 = bitcast i32* %.omp.lb to i8*
  store i32 0, i32* %.omp.lb, align 4
  %1 = bitcast i32* %.omp.ub to i8*
  store i32 0, i32* %.omp.ub, align 4
  br label %DIR.OMP.PARALLEL.LOOP.1

DIR.OMP.PARALLEL.LOOP.1:
  br label %DIR.OMP.PARALLEL.LOOP.19

DIR.OMP.PARALLEL.LOOP.19:
; CHECK-DEFAULT-NOT:             call i32 @llvm.csa.spmdization.entry(
; CHECK-WORKERS:  [[SPMD:%.*]] = call i32 @llvm.csa.spmdization.entry(i32 2, i32 0)
; CHECK-MODE:     [[SPMD:%.*]] = call i32 @llvm.csa.spmdization.entry(i32 3, i32 4)
; CHECK-COMMON: [[REGION:%.*]] = call i32 @llvm.csa.parallel.region.entry(i32 {{[0-9]+}})
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"() ]
  br label %DIR.OMP.PARALLEL.LOOP.2

DIR.OMP.PARALLEL.LOOP.2:
  %3 = load i32, i32* %.omp.lb, align 4
  %4 = load i32, i32* %.omp.ub, align 4
  %5 = bitcast i32* %i to i8*
  %cmp7 = icmp sle i32 %3, %4
  br i1 %cmp7, label %omp.inner.for.body.lr.ph, label %DIR.OMP.END.PARALLEL.LOOP.3

omp.inner.for.body.lr.ph:
  br label %omp.inner.for.body

omp.inner.for.body:
  %.omp.iv.08 = phi i32 [ %3, %omp.inner.for.body.lr.ph ], [ %add1, %omp.inner.for.body ]
; CHECK-COMMON: [[SECTION:%.*]] = call i32 @llvm.csa.parallel.section.entry(i32 [[REGION]])
  store i32 %.omp.iv.08, i32* %i, align 4
; CHECK-COMMON: call void @llvm.csa.parallel.section.exit(i32 [[SECTION]])
  %add1 = add nsw i32 %.omp.iv.08, 1
  %cmp = icmp sle i32 %add1, %4
  br i1 %cmp, label %omp.inner.for.body, label %DIR.OMP.END.PARALLEL.LOOP.3

DIR.OMP.END.PARALLEL.LOOP.3:
  br label %DIR.OMP.END.PARALLEL.LOOP.310

DIR.OMP.END.PARALLEL.LOOP.310:
; CHECK-COMMON:      call void @llvm.csa.parallel.region.exit(i32 [[REGION]])
; CHECK-DEFAULT-NOT: call void @llvm.csa.spmdization.exit(
; CHECK-WORKERS:     call void @llvm.csa.spmdization.exit(i32 [[SPMD]])
; CHECK-MODE:        call void @llvm.csa.spmdization.exit(i32 [[SPMD]])
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.LOOP.4

DIR.OMP.END.PARALLEL.LOOP.4:
  ret void
}

; CHECK-LABEL: @foo.num_threads.no.schedule
define void @foo.num_threads.no.schedule() {
entry:
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %0 = bitcast i32* %.omp.lb to i8*
  store i32 0, i32* %.omp.lb, align 4
  %1 = bitcast i32* %.omp.ub to i8*
  store i32 0, i32* %.omp.ub, align 4
  br label %DIR.OMP.PARALLEL.LOOP.1

DIR.OMP.PARALLEL.LOOP.1:
  br label %DIR.OMP.PARALLEL.LOOP.19

DIR.OMP.PARALLEL.LOOP.19:
; CHECK-DEFAULT:  [[SPMD:%.*]] = call i32 @llvm.csa.spmdization.entry(i32 5, i32 0)
; CHECK-WORKERS:  [[SPMD:%.*]] = call i32 @llvm.csa.spmdization.entry(i32 5, i32 0)
; CHECK-MODE:     [[SPMD:%.*]] = call i32 @llvm.csa.spmdization.entry(i32 5, i32 4)
; CHECK-COMMON: [[REGION:%.*]] = call i32 @llvm.csa.parallel.region.entry(i32 {{[0-9]+}})
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.NUM_THREADS"(i32 5) ]
  br label %DIR.OMP.PARALLEL.LOOP.2

DIR.OMP.PARALLEL.LOOP.2:
  %3 = load i32, i32* %.omp.lb, align 4
  %4 = load i32, i32* %.omp.ub, align 4
  %5 = bitcast i32* %i to i8*
  %cmp7 = icmp sle i32 %3, %4
  br i1 %cmp7, label %omp.inner.for.body.lr.ph, label %DIR.OMP.END.PARALLEL.LOOP.3

omp.inner.for.body.lr.ph:
  br label %omp.inner.for.body

omp.inner.for.body:
  %.omp.iv.08 = phi i32 [ %3, %omp.inner.for.body.lr.ph ], [ %add1, %omp.inner.for.body ]
; CHECK-COMMON: [[SECTION:%.*]] = call i32 @llvm.csa.parallel.section.entry(i32 [[REGION]])
  store i32 %.omp.iv.08, i32* %i, align 4
; CHECK-COMMON: call void @llvm.csa.parallel.section.exit(i32 [[SECTION]])
  %add1 = add nsw i32 %.omp.iv.08, 1
  %cmp = icmp sle i32 %add1, %4
  br i1 %cmp, label %omp.inner.for.body, label %DIR.OMP.END.PARALLEL.LOOP.3

DIR.OMP.END.PARALLEL.LOOP.3:
  br label %DIR.OMP.END.PARALLEL.LOOP.310

DIR.OMP.END.PARALLEL.LOOP.310:
; CHECK-COMMON:      call void @llvm.csa.parallel.region.exit(i32 [[REGION]])
; CHECK-COMMON:      call void @llvm.csa.spmdization.exit(i32 [[SPMD]])
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.LOOP.4

DIR.OMP.END.PARALLEL.LOOP.4:
  ret void
}

; CHECK-LABEL: @foo.num_threads.schedule
define void @foo.num_threads.schedule() {
entry:
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %0 = bitcast i32* %.omp.lb to i8*
  store i32 0, i32* %.omp.lb, align 4
  %1 = bitcast i32* %.omp.ub to i8*
  store i32 0, i32* %.omp.ub, align 4
  br label %DIR.OMP.PARALLEL.LOOP.1

DIR.OMP.PARALLEL.LOOP.1:
  br label %DIR.OMP.PARALLEL.LOOP.19

DIR.OMP.PARALLEL.LOOP.19:
; CHECK-COMMON:   [[SPMD:%.*]] = call i32 @llvm.csa.spmdization.entry(i32 3, i32 0)
; CHECK-COMMON: [[REGION:%.*]] = call i32 @llvm.csa.parallel.region.entry(i32 {{[0-9]+}})
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.NUM_THREADS"(i32 3), "QUAL.OMP.SCHEDULE.STATIC"(i32 0) ]
  br label %DIR.OMP.PARALLEL.LOOP.2

DIR.OMP.PARALLEL.LOOP.2:
  %3 = load i32, i32* %.omp.lb, align 4
  %4 = load i32, i32* %.omp.ub, align 4
  %5 = bitcast i32* %i to i8*
  %cmp7 = icmp sle i32 %3, %4
  br i1 %cmp7, label %omp.inner.for.body.lr.ph, label %DIR.OMP.END.PARALLEL.LOOP.3

omp.inner.for.body.lr.ph:
  br label %omp.inner.for.body

omp.inner.for.body:
  %.omp.iv.08 = phi i32 [ %3, %omp.inner.for.body.lr.ph ], [ %add1, %omp.inner.for.body ]
; CHECK-COMMON: [[SECTION:%.*]] = call i32 @llvm.csa.parallel.section.entry(i32 [[REGION]])
  store i32 %.omp.iv.08, i32* %i, align 4
; CHECK-COMMON: call void @llvm.csa.parallel.section.exit(i32 [[SECTION]])
  %add1 = add nsw i32 %.omp.iv.08, 1
  %cmp = icmp sle i32 %add1, %4
  br i1 %cmp, label %omp.inner.for.body, label %DIR.OMP.END.PARALLEL.LOOP.3

DIR.OMP.END.PARALLEL.LOOP.3:
  br label %DIR.OMP.END.PARALLEL.LOOP.310

DIR.OMP.END.PARALLEL.LOOP.310:
; CHECK-COMMON:      call void @llvm.csa.parallel.region.exit(i32 [[REGION]])
; CHECK-COMMON:      call void @llvm.csa.spmdization.exit(i32 [[SPMD]])
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.LOOP.4

DIR.OMP.END.PARALLEL.LOOP.4:
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

