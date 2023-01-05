; INTEL_FEATURE_CSA
; RUN: opt -passes="require<vpo-wrninfo>,require<vpo-wrncollection>,require<domtree>,vpo-paropt" < %s -loopopt-use-omp-region -lcssa-verification -S | FileCheck %s
; REQUIRES: csa-registered-target
;
; Check that paropt retains omp directives for parallel loops with one thread
; with -loopopt-use-omp-region.

target triple = "csa"

; CHECK-LABEL: @foo.one.thread
define void @foo.one.thread() {
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
; CHECK: [[TOK:%.*]] = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.NUM_THREADS"(i32 1), "QUAL.OMP.FIRSTPRIVATE"(i32* null), "QUAL.OMP.PRIVATE"(i32* null)
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.NUM_THREADS"(i32 1), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %i) ]
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
; CHECK: call void @llvm.directive.region.exit(token [[TOK]]) [ "DIR.OMP.END.PARALLEL.LOOP"()
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.LOOP.4

DIR.OMP.END.PARALLEL.LOOP.4:
  ret void
}

; CHECK-LABEL: @foo.two.threads
define void @foo.two.threads() {
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
; CHECK-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.NUM_THREADS"(i32 2), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %i) ]
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
; CHECK-NOT: call void @llvm.directive.region.exit(token {{.*}}) [ "DIR.OMP.END.PARALLEL.LOOP"()
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.LOOP.4

DIR.OMP.END.PARALLEL.LOOP.4:
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
; end INTEL_FEATURE_CSA
