; INTEL_FEATURE_CSA
; RUN: opt -passes="require<vpo-wrninfo>,require<vpo-wrncollection>,require<domtree>,vpo-paropt" < %s -lcssa-verification -S | FileCheck %s
; REQUIRES: csa-registered-target
;
; Check paropt lowering of "omp parallel for" for CSA target.

target triple = "csa"

; CHECK-LABEL: @foo.no.schedule
define void @foo.no.schedule() {
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
; No shedule translates to "blocked" SPMDization mode (0).
; CHECK: [[SPMD:%.*]] = call i32 @llvm.csa.spmdization.entry(i32 2, i32 0)
; CHECK: [[REGION:%.*]] = call i32 @llvm.csa.parallel.region.entry(i32 {{[0-9]+}})
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.NUM_THREADS"(i32 2) ]
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
; CHECK: [[SECTION:%.*]] = call i32 @llvm.csa.parallel.section.entry(i32 [[REGION]])
  store i32 %.omp.iv.08, i32* %i, align 4
; CHECK: call void @llvm.csa.parallel.section.exit(i32 [[SECTION]])
  %add1 = add nsw i32 %.omp.iv.08, 1
  %cmp = icmp sle i32 %add1, %4
  br i1 %cmp, label %omp.inner.for.body, label %DIR.OMP.END.PARALLEL.LOOP.3

DIR.OMP.END.PARALLEL.LOOP.3:
  br label %DIR.OMP.END.PARALLEL.LOOP.310

DIR.OMP.END.PARALLEL.LOOP.310:
; CHECK: call void @llvm.csa.parallel.region.exit(i32 [[REGION]])
; CHECK: call void @llvm.csa.spmdization.exit(i32 [[SPMD]])
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.LOOP.4

DIR.OMP.END.PARALLEL.LOOP.4:
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

; CHECK-LABEL: @foo.shedule.static.no.chunk
define void @foo.shedule.static.no.chunk() {
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
; Static schedule (no chunk size) translates to blocked SPMDization mode (0).
; CHECK: [[SPMD:%.*]] = call i32 @llvm.csa.spmdization.entry(i32 2, i32 0)
; CHECK: [[REGION:%.*]] = call i32 @llvm.csa.parallel.region.entry(i32 {{[0-9]+}})
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.NUM_THREADS"(i32 2), "QUAL.OMP.SCHEDULE.STATIC"(i32 0) ]
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
; CHECK: [[SECTION:%.*]] = call i32 @llvm.csa.parallel.section.entry(i32 [[REGION]])
  store i32 %.omp.iv.08, i32* %i, align 4
; CHECK: call void @llvm.csa.parallel.section.exit(i32 [[SECTION]])
  %add1 = add nsw i32 %.omp.iv.08, 1
  %cmp = icmp sle i32 %add1, %4
  br i1 %cmp, label %omp.inner.for.body, label %DIR.OMP.END.PARALLEL.LOOP.3

DIR.OMP.END.PARALLEL.LOOP.3:
  br label %DIR.OMP.END.PARALLEL.LOOP.310

DIR.OMP.END.PARALLEL.LOOP.310:
; CHECK: call void @llvm.csa.parallel.region.exit(i32 [[REGION]])
; CHECK: call void @llvm.csa.spmdization.exit(i32 [[SPMD]])
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.LOOP.4

DIR.OMP.END.PARALLEL.LOOP.4:
  ret void
}

; CHECK-LABEL: @foo.shedule.static.1
define void @foo.shedule.static.1() {
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
; Static schedule with chunksize = 1 translates to "cyclic" SPMDization mode (1).
; CHECK: [[SPMD:%.*]] = call i32 @llvm.csa.spmdization.entry(i32 2, i32 1)
; CHECK: [[REGION:%.*]] = call i32 @llvm.csa.parallel.region.entry(i32 {{[0-9]+}})
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
; CHECK: [[SECTION:%.*]] = call i32 @llvm.csa.parallel.section.entry(i32 [[REGION]])
  store i32 %.omp.iv.08, i32* %i, align 4
; CHECK: call void @llvm.csa.parallel.section.exit(i32 [[SECTION]])
  %add1 = add nsw i32 %.omp.iv.08, 1
  %cmp = icmp sle i32 %add1, %4
  br i1 %cmp, label %omp.inner.for.body, label %DIR.OMP.END.PARALLEL.LOOP.3

DIR.OMP.END.PARALLEL.LOOP.3:
  br label %DIR.OMP.END.PARALLEL.LOOP.310

DIR.OMP.END.PARALLEL.LOOP.310:
; CHECK: call void @llvm.csa.parallel.region.exit(i32 [[REGION]])
; CHECK: call void @llvm.csa.spmdization.exit(i32 [[SPMD]])
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.LOOP.4

DIR.OMP.END.PARALLEL.LOOP.4:
  ret void
}

; CHECK-LABEL: @foo.shedule.static.4
define void @foo.shedule.static.4() {
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
; Static schedule with chunksize > 1 translates to hybrid SPMDization mode (chunksize).
; CHECK: [[SPMD:%.*]] = call i32 @llvm.csa.spmdization.entry(i32 2, i32 4)
; CHECK: [[REGION:%.*]] = call i32 @llvm.csa.parallel.region.entry(i32 {{[0-9]+}})
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.NUM_THREADS"(i32 2), "QUAL.OMP.SCHEDULE.STATIC"(i32 4) ]
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
; CHECK: [[SECTION:%.*]] = call i32 @llvm.csa.parallel.section.entry(i32 [[REGION]])
  store i32 %.omp.iv.08, i32* %i, align 4
; CHECK: call void @llvm.csa.parallel.section.exit(i32 [[SECTION]])
  %add1 = add nsw i32 %.omp.iv.08, 1
  %cmp = icmp sle i32 %add1, %4
  br i1 %cmp, label %omp.inner.for.body, label %DIR.OMP.END.PARALLEL.LOOP.3

DIR.OMP.END.PARALLEL.LOOP.3:
  br label %DIR.OMP.END.PARALLEL.LOOP.310

DIR.OMP.END.PARALLEL.LOOP.310:
; CHECK: call void @llvm.csa.parallel.region.exit(i32 [[REGION]])
; CHECK: call void @llvm.csa.spmdization.exit(i32 [[SPMD]])
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.LOOP.4

DIR.OMP.END.PARALLEL.LOOP.4:
  ret void
}

; CHECK-LABEL: @foo.par.and.loop
define void @foo.par.and.loop() {
entry:
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %0 = bitcast i32* %.omp.lb to i8*
  store i32 0, i32* %.omp.lb, align 4
  %1 = bitcast i32* %.omp.ub to i8*
  store i32 0, i32* %.omp.ub, align 4
  br label %DIR.OMP.PARALLEL

DIR.OMP.PARALLEL:
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  br label %DIR.OMP.LOOP

DIR.OMP.LOOP:
; CHECK: [[REGION:%.*]] = call i32 @llvm.csa.parallel.region.entry(i32 {{[0-9]+}})
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"() ]
  br label %DIR.OMP.LOOP.1

DIR.OMP.LOOP.1:
  %4 = load i32, i32* %.omp.lb, align 4
  %5 = load i32, i32* %.omp.ub, align 4
  %cmp7 = icmp sle i32 %4, %5
  br i1 %cmp7, label %omp.inner.for.body.lr.ph, label %DIR.OMP.END.LOOP

omp.inner.for.body.lr.ph:
  br label %omp.inner.for.body

omp.inner.for.body:
  %.omp.iv.08 = phi i32 [ %4, %omp.inner.for.body.lr.ph ], [ %add1, %omp.inner.for.body ]
; CHECK: [[SECTION:%.*]] = call i32 @llvm.csa.parallel.section.entry(i32 [[REGION]])
  store i32 %.omp.iv.08, i32* %i, align 4
  %add1 = add nsw i32 %.omp.iv.08, 1
  %cmp = icmp sle i32 %add1, %5
; CHECK: call void @llvm.csa.parallel.section.exit(i32 [[SECTION]])
  br i1 %cmp, label %omp.inner.for.body, label %DIR.OMP.END.LOOP

DIR.OMP.END.LOOP:
; CHECK: call void @llvm.csa.parallel.region.exit(i32 [[REGION]])
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL

DIR.OMP.END.PARALLEL:
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]
  br label %exit

exit:
  ret void
}

; CHECK-LABEL: @foo.par.and.loop.and.loop
define void @foo.par.and.loop.and.loop() {
entry:
  %.omp.lb1 = alloca i32, align 4
  %.omp.ub1 = alloca i32, align 4
  %i1 = alloca i32, align 4
  %0 = bitcast i32* %.omp.lb1 to i8*
  store i32 0, i32* %.omp.lb1, align 4
  %1 = bitcast i32* %.omp.ub1 to i8*
  store i32 0, i32* %.omp.ub1, align 4
  %.omp.lb2 = alloca i32, align 4
  %.omp.ub2 = alloca i32, align 4
  %i2 = alloca i32, align 4
  %2 = bitcast i32* %.omp.lb2 to i8*
  store i32 0, i32* %.omp.lb2, align 4
  %3 = bitcast i32* %.omp.ub2 to i8*
  store i32 0, i32* %.omp.ub2, align 4
  br label %DIR.OMP.PARALLEL

DIR.OMP.PARALLEL:
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  br label %DIR.OMP.LOOP1

DIR.OMP.LOOP1:
; CHECK: [[REGION1:%.*]] = call i32 @llvm.csa.parallel.region.entry(i32 {{[0-9]+}})
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"() ]
  br label %DIR.OMP.LOOP.11

DIR.OMP.LOOP.11:
  %6 = load i32, i32* %.omp.lb1, align 4
  %7 = load i32, i32* %.omp.ub1, align 4
  %cmp71 = icmp sle i32 %6, %7
  br i1 %cmp71, label %omp.inner.for.body.lr.ph1, label %DIR.OMP.END.LOOP1

omp.inner.for.body.lr.ph1:
  br label %omp.inner.for.body1

omp.inner.for.body1:
  %.omp.iv.081 = phi i32 [ %6, %omp.inner.for.body.lr.ph1 ], [ %add11, %omp.inner.for.body1 ]
; CHECK: [[SECTION1:%.*]] = call i32 @llvm.csa.parallel.section.entry(i32 [[REGION1]])
  store i32 %.omp.iv.081, i32* %i1, align 4
  %add11 = add nsw i32 %.omp.iv.081, 1
  %cmp1 = icmp sle i32 %add11, %7
; CHECK: call void @llvm.csa.parallel.section.exit(i32 [[SECTION1]])
  br i1 %cmp1, label %omp.inner.for.body1, label %DIR.OMP.END.LOOP1

DIR.OMP.END.LOOP1:
; CHECK: call void @llvm.csa.parallel.region.exit(i32 [[REGION1]])
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.LOOP"() ]
  br label %DIR.OMP.LOOP2

DIR.OMP.LOOP2:
; CHECK: [[REGION2:%.*]] = call i32 @llvm.csa.parallel.region.entry(i32 {{[0-9]+}})
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"() ]
  br label %DIR.OMP.LOOP.12

DIR.OMP.LOOP.12:
  %9 = load i32, i32* %.omp.lb2, align 4
  %10 = load i32, i32* %.omp.ub2, align 4
  %cmp72 = icmp sle i32 %9, %10
  br i1 %cmp72, label %omp.inner.for.body.lr.ph2, label %DIR.OMP.END.LOOP2

omp.inner.for.body.lr.ph2:
  br label %omp.inner.for.body2

omp.inner.for.body2:
  %.omp.iv.082 = phi i32 [ %9, %omp.inner.for.body.lr.ph2 ], [ %add12, %omp.inner.for.body2 ]
; CHECK: [[SECTION2:%.*]] = call i32 @llvm.csa.parallel.section.entry(i32 [[REGION2]])
  store i32 %.omp.iv.082, i32* %i2, align 4
  %add12 = add nsw i32 %.omp.iv.082, 1
  %cmp2 = icmp sle i32 %add12, %10
; CHECK: call void @llvm.csa.parallel.section.exit(i32 [[SECTION2]])
  br i1 %cmp2, label %omp.inner.for.body2, label %DIR.OMP.END.LOOP2

DIR.OMP.END.LOOP2:
; CHECK: call void @llvm.csa.parallel.region.exit(i32 [[REGION2]])
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL

DIR.OMP.END.PARALLEL:
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL"() ]
  br label %exit

exit:
  ret void
}
; end INTEL_FEATURE_CSA
