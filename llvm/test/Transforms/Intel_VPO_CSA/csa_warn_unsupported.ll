; INTEL_FEATURE_CSA
; RUN: opt -passes="require<vpo-wrninfo>,require<vpo-wrncollection>,require<domtree>,vpo-paropt" < %s -lcssa-verification -S 2>&1 | FileCheck %s
; REQUIRES: csa-registered-target
;
target triple = "csa"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define void @f1(i32 %x) {
entry:
  br label %DIR.OMP.PARALLEL.LOOP.19

DIR.OMP.PARALLEL.LOOP.19:
; CHECK: warning:{{.*}}CSA - schedule chunk must be a compile time constant
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.STATIC"(i32 %x) ]
  br label %DIR.OMP.PARALLEL.LOOP.2

DIR.OMP.PARALLEL.LOOP.2:
  %cmp7 = icmp sle i32 0, 0
  br i1 %cmp7, label %omp.inner.for.body.lr.ph, label %DIR.OMP.END.PARALLEL.LOOP.3

omp.inner.for.body.lr.ph:
  br label %omp.inner.for.body

omp.inner.for.body:
  %.omp.iv.08 = phi i32 [ 0, %omp.inner.for.body.lr.ph ], [ %add1, %omp.inner.for.body ]
  %add1 = add nsw i32 %.omp.iv.08, 1
  %cmp = icmp sle i32 %add1, 0
  br i1 %cmp, label %omp.inner.for.body, label %DIR.OMP.END.PARALLEL.LOOP.3

DIR.OMP.END.PARALLEL.LOOP.3:
  br label %DIR.OMP.END.PARALLEL.LOOP.310

DIR.OMP.END.PARALLEL.LOOP.310:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.LOOP.4

DIR.OMP.END.PARALLEL.LOOP.4:
  ret void
}

define void @f2() {
entry:
  br label %DIR.OMP.PARALLEL.LOOP.19

DIR.OMP.PARALLEL.LOOP.19:
; CHECK: warning:{{.*}}CSA - ignoring unsupported schedule type
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.DYNAMIC"(i32 1) ]
  br label %DIR.OMP.PARALLEL.LOOP.2

DIR.OMP.PARALLEL.LOOP.2:
  %cmp7 = icmp sle i32 0, 0
  br i1 %cmp7, label %omp.inner.for.body.lr.ph, label %DIR.OMP.END.PARALLEL.LOOP.3

omp.inner.for.body.lr.ph:
  br label %omp.inner.for.body

omp.inner.for.body:
  %.omp.iv.08 = phi i32 [ 0, %omp.inner.for.body.lr.ph ], [ %add1, %omp.inner.for.body ]
  %add1 = add nsw i32 %.omp.iv.08, 1
  %cmp = icmp sle i32 %add1, 0
  br i1 %cmp, label %omp.inner.for.body, label %DIR.OMP.END.PARALLEL.LOOP.3

DIR.OMP.END.PARALLEL.LOOP.3:
  br label %DIR.OMP.END.PARALLEL.LOOP.310

DIR.OMP.END.PARALLEL.LOOP.310:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.LOOP.4

DIR.OMP.END.PARALLEL.LOOP.4:
  ret void
}

define void @f3() {
entry:
  br label %DIR.OMP.PARALLEL.LOOP.19

DIR.OMP.PARALLEL.LOOP.19:
; CHECK: warning:{{.*}}CSA - ignoring unsupported schedule type
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.GUIDED"(i32 1) ]
  br label %DIR.OMP.PARALLEL.LOOP.2

DIR.OMP.PARALLEL.LOOP.2:
  %cmp7 = icmp sle i32 0, 0
  br i1 %cmp7, label %omp.inner.for.body.lr.ph, label %DIR.OMP.END.PARALLEL.LOOP.3

omp.inner.for.body.lr.ph:
  br label %omp.inner.for.body

omp.inner.for.body:
  %.omp.iv.08 = phi i32 [ 0, %omp.inner.for.body.lr.ph ], [ %add1, %omp.inner.for.body ]
  %add1 = add nsw i32 %.omp.iv.08, 1
  %cmp = icmp sle i32 %add1, 0
  br i1 %cmp, label %omp.inner.for.body, label %DIR.OMP.END.PARALLEL.LOOP.3

DIR.OMP.END.PARALLEL.LOOP.3:
  br label %DIR.OMP.END.PARALLEL.LOOP.310

DIR.OMP.END.PARALLEL.LOOP.310:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.LOOP.4

DIR.OMP.END.PARALLEL.LOOP.4:
  ret void
}

define void @f4() {
entry:
  br label %DIR.OMP.PARALLEL.LOOP.19

DIR.OMP.PARALLEL.LOOP.19:
; CHECK: warning:{{.*}}CSA - ignoring unsupported schedule type
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.RUNTIME"(i32 0) ]
  br label %DIR.OMP.PARALLEL.LOOP.2

DIR.OMP.PARALLEL.LOOP.2:
  %cmp7 = icmp sle i32 0, 0
  br i1 %cmp7, label %omp.inner.for.body.lr.ph, label %DIR.OMP.END.PARALLEL.LOOP.3

omp.inner.for.body.lr.ph:
  br label %omp.inner.for.body

omp.inner.for.body:
  %.omp.iv.08 = phi i32 [ 0, %omp.inner.for.body.lr.ph ], [ %add1, %omp.inner.for.body ]
  %add1 = add nsw i32 %.omp.iv.08, 1
  %cmp = icmp sle i32 %add1, 0
  br i1 %cmp, label %omp.inner.for.body, label %DIR.OMP.END.PARALLEL.LOOP.3

DIR.OMP.END.PARALLEL.LOOP.3:
  br label %DIR.OMP.END.PARALLEL.LOOP.310

DIR.OMP.END.PARALLEL.LOOP.310:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.LOOP.4

DIR.OMP.END.PARALLEL.LOOP.4:
  ret void
}

define void @f5(i32 %x) {
entry:
  br label %DIR.OMP.PARALLEL.LOOP.19

DIR.OMP.PARALLEL.LOOP.19:
; CHECK: warning:{{.*}}CSA - num_threads must be a compile time constant
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.NUM_THREADS"(i32 %x) ]
  br label %DIR.OMP.PARALLEL.LOOP.2

DIR.OMP.PARALLEL.LOOP.2:
  %cmp7 = icmp sle i32 0, 0
  br i1 %cmp7, label %omp.inner.for.body.lr.ph, label %DIR.OMP.END.PARALLEL.LOOP.3

omp.inner.for.body.lr.ph:
  br label %omp.inner.for.body

omp.inner.for.body:
  %.omp.iv.08 = phi i32 [ 0, %omp.inner.for.body.lr.ph ], [ %add1, %omp.inner.for.body ]
  %add1 = add nsw i32 %.omp.iv.08, 1
  %cmp = icmp sle i32 %add1, 0
  br i1 %cmp, label %omp.inner.for.body, label %DIR.OMP.END.PARALLEL.LOOP.3

DIR.OMP.END.PARALLEL.LOOP.3:
  br label %DIR.OMP.END.PARALLEL.LOOP.310

DIR.OMP.END.PARALLEL.LOOP.310:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.LOOP.4

DIR.OMP.END.PARALLEL.LOOP.4:
  ret void
}
; end INTEL_FEATURE_CSA
