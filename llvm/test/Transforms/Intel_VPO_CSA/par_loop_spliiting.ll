; INTEL_FEATURE_CSA
; RUN: opt -vpo-paropt -csa-omp-paropt-loop-splitting -S %s| FileCheck %s
; REQUIRES: csa-registered-target
;
; Check lowering of "omp parallel for" construct for CSA with paropt
; spmdization.

target triple = "csa"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

; CHECK-LABEL: @foo.cyclic
define void @foo.cyclic() {
entry:
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %0 = bitcast i32* %.omp.lb to i8*
  store i32 0, i32* %.omp.lb, align 4
  %1 = bitcast i32* %.omp.ub to i8*
  store i32 0, i32* %.omp.ub, align 4
  br label %DIR.OMP.PARALLEL.LOOP.1

; Check that paropt creates two worker loops annotated by parellel region/section
; entry/exit calls that are also nested in a parallel region.

; CHECK-DAG: [[REGION:%.*]] = call i32 @llvm.csa.parallel.region.entry(i32 {{[0-9]+}})
; CHECK-DAG:   [[SECTION_W0:%.*]] = call i32 @llvm.csa.parallel.section.entry(i32 [[REGION]])
; CHECK-DAG:     [[REGION_W0_LOOP:%.*]] = call i32 @llvm.csa.parallel.region.entry(i32 {{[0-9]+}})
; CHECK-DAG:       [[SECTION_W0_LOOP_BODY:%.*]] = call i32 @llvm.csa.parallel.section.entry(i32 [[REGION_W0_LOOP]])
; CHECK-DAG:       call void @llvm.csa.parallel.section.exit(i32 [[SECTION_W0_LOOP_BODY]])
; CHECK-DAG:     call void @llvm.csa.parallel.region.exit(i32 [[REGION_W0_LOOP]])
; CHECK-DAG:   call void @llvm.csa.parallel.section.exit(i32 [[SECTION_W0]])
; CHECK-DAG:   [[SECTION_W1:%.*]] = call i32 @llvm.csa.parallel.section.entry(i32 [[REGION]])
; CHECK-DAG:     [[REGION_W1_LOOP:%.*]] = call i32 @llvm.csa.parallel.region.entry(i32 {{[0-9]+}})
; CHECK-DAG:       [[SECTION_W1_LOOP_BODY:%.*]] = call i32 @llvm.csa.parallel.section.entry(i32 [[REGION_W1_LOOP]])
; CHECK-DAG:       call void @llvm.csa.parallel.section.exit(i32 [[SECTION_W1_LOOP_BODY]])
; CHECK-DAG:     call void @llvm.csa.parallel.region.exit(i32 [[REGION_W1_LOOP]])
; CHECK-DAG:   call void @llvm.csa.parallel.section.exit(i32 [[SECTION_W1]])
; CHECK-DAG: call void @llvm.csa.parallel.region.exit(i32 [[REGION]])

DIR.OMP.PARALLEL.LOOP.1:
  br label %DIR.OMP.PARALLEL.LOOP.19

DIR.OMP.PARALLEL.LOOP.19:
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.NUM_THREADS"(i32 2), "QUAL.OMP.SCHEDULE.STATIC"(i32 1) ]
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
  store i32 %.omp.iv.08, i32* %i, align 4
  %add1 = add nsw i32 %.omp.iv.08, 1
  %cmp = icmp sle i32 %add1, %4
  br i1 %cmp, label %omp.inner.for.body, label %DIR.OMP.END.PARALLEL.LOOP.3

DIR.OMP.END.PARALLEL.LOOP.3:
  br label %DIR.OMP.END.PARALLEL.LOOP.310

DIR.OMP.END.PARALLEL.LOOP.310:
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.LOOP.4

DIR.OMP.END.PARALLEL.LOOP.4:
  ret void
}
; end INTEL_FEATURE_CSA
