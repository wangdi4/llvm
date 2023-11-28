; This test is checking if we correctly bailout vectorization for double nested SIMD loops.

; RUN: opt -passes=hir-ssa-deconstruction,hir-vplan-vec -debug-only=LoopVectorizationPlanner -disable-output -vplan-nested-simd-strategy=bailout < %s 2>&1 | FileCheck %s
; RUN: opt -passes=hir-ssa-deconstruction,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter -disable-output -vplan-nested-simd-strategy=bailout -intel-opt-report=medium < %s 2>&1 | FileCheck %s --check-prefix=OPTRPTMED

; CHECK: Unsupported nested OpenMP (simd) loop or region.
; OPTRPTMED: remark #15574: HIR: simd loop was not vectorized: unsupported nested OpenMP (simd) loop or region.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @_Z3fooiiPfS_S_(i32 %n, i32 %m) {
entry:
  %l1.linear.iv = alloca i64, align 8
  %cmp31 = icmp sgt i32 %m, 0
  br i1 %cmp31, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %l1.linear.iv, i64 0, i32 1, i32 1) ]
  %cmp4 = icmp sgt i32 %n, 0
  %wide.trip.count3638 = zext i32 %m to i64
  %wide.trip.count39 = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %omp.precond.end
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %omp.precond.end, %for.body.lr.ph
  %indvars.iv34 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next35, %omp.precond.end ]
  br i1 %cmp4, label %DIR.OMP.SIMD.2, label %omp.precond.end

DIR.OMP.SIMD.2:                                   ; preds = %for.body
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  %ld = load i64, ptr %l1.linear.iv, align 8
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.2
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %omp.inner.for.body ]
  call void @_Z3bazPllll(i64 %ld)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count39
  br i1 %exitcond, label %omp.inner.for.cond.omp.loop.exit.split_crit_edge.split.split, label %omp.inner.for.body

omp.inner.for.cond.omp.loop.exit.split_crit_edge.split.split: ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.inner.for.cond.omp.loop.exit.split_crit_edge.split.split, %for.body
  %indvars.iv.next35 = add nuw nsw i64 %indvars.iv34, 1
  %exitcond37 = icmp eq i64 %indvars.iv.next35, %wide.trip.count3638
  br i1 %exitcond37, label %for.cond.cleanup.loopexit, label %for.body
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

declare void @_Z3bazPllll(i64)
