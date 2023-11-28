; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -passes="function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,function(simplifycfg,instcombine),hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-vplan-vec" -disable-output -print-before=hir-vplan-vec %s 2>&1 | FileCheck %s

; VPlanDriverHIR cannot be run via new pass manager.

; Original code:
; void foo(int *x, int *y) {
; #pragma omp parallel for simd
;   for (int i = 0; i < 10000; ++i)
;     x[i] = y[i] + i;
; }

; Check that the HIR loop region does not contain any code
; generated during parallel loop outlining.

; CHECK: [[RID:%.+]] = @llvm.directive.region.entry(){{.*}}DIR.OMP.SIMD()
; CHECK-NOT: __kmpc_for_static_init
; CHECK-NOT: __kmpc_for_static_fini
; CHECK: @llvm.directive.region.exit([[RID]]){{.*}}DIR.OMP.END.SIMD()

; ModuleID = 'simd_dirs_sinking.c'
source_filename = "simd_dirs_sinking.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr noundef %x, ptr noundef %y) {
entry:
  %x.addr = alloca ptr, align 8
  %y.addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr %x, ptr %x.addr, align 8
  store ptr %y, ptr %y.addr, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 9999, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %x.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %y.addr, ptr null, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i, i32 0, i32 1, i32 1) ]

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
  %6 = load ptr, ptr %y.addr, align 8
  %7 = load i32, ptr %i, align 4
  %idxprom = sext i32 %7 to i64
  %arrayidx = getelementptr inbounds i32, ptr %6, i64 %idxprom
  %8 = load i32, ptr %arrayidx, align 4
  %9 = load i32, ptr %i, align 4
  %add1 = add nsw i32 %8, %9
  %10 = load ptr, ptr %x.addr, align 8
  %11 = load i32, ptr %i, align 4
  %idxprom2 = sext i32 %11 to i64
  %arrayidx3 = getelementptr inbounds i32, ptr %10, i64 %idxprom2
  store i32 %add1, ptr %arrayidx3, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, ptr %.omp.iv, align 4
  %add4 = add nsw i32 %12, 1
  store i32 %add4, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

; end INTEL_FEATURE_SW_ADVANCED

