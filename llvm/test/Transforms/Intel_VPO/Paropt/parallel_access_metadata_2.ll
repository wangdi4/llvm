; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -simplifycfg -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,function(simplifycfg)' -S %s | FileCheck %s
;
; This test checks that paropt pass does not add parallel access metadata to
; loops that do not have non-monotonic property.
;
; void test1(float *P) {
; #pragma omp for ordered
;   for (int I = 0 ; I < 100; I++)
;     P[I] = I;
; }
;
; void test2(float *P) {
; #pragma omp for schedule(monotonic: auto)
;   for (int I = 0 ; I < 100; I++)
;     P[I] = I;
; }
;
; void test3(float *P) {
; #pragma omp for schedule(static)
;   for (int I = 0 ; I < 100; I++)
;     P[I] = I;
; }
;
; void test4(float *P) {
; #pragma omp simd safelen(4)
;   for (int I = 0 ; I < 100; I++)
;     P[I] = I;
; }
;
; CHECK-NOT: llvm.access.group
; CHECK-NOT: llvm.loop.parallel_accesses
;
; INTEL_CUSTOMIZATION
; CHECK-NOT: llvm.loop.vectorize.ivdep_loop
; end INTEL_CUSTOMIZATION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @test1(ptr noundef %P) {
entry:
  %P.addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store ptr %P, ptr %P.addr, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.ORDERED"(i32 0),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %I, i32 0, i32 1) ]

  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %I, align 4
  %5 = load i32, ptr %I, align 4
  %conv = sitofp i32 %5 to float
  %6 = load ptr, ptr %P.addr, align 8
  %7 = load i32, ptr %I, align 4
  %idxprom = sext i32 %7 to i64
  %arrayidx = getelementptr inbounds float, ptr %6, i64 %idxprom
  store float %conv, ptr %arrayidx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %8, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token) 

define dso_local void @test2(ptr noundef %P) {
entry:
  %P.addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store ptr %P, ptr %P.addr, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.SCHEDULE.AUTO:MONOTONIC"(i32 0),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %I, i32 0, i32 1) ]

  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %I, align 4
  %5 = load i32, ptr %I, align 4
  %conv = sitofp i32 %5 to float
  %6 = load ptr, ptr %P.addr, align 8
  %7 = load i32, ptr %I, align 4
  %idxprom = sext i32 %7 to i64
  %arrayidx = getelementptr inbounds float, ptr %6, i64 %idxprom
  store float %conv, ptr %arrayidx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %8, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]

  ret void
}

define dso_local void @test3(ptr noundef %P) {
entry:
  %P.addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store ptr %P, ptr %P.addr, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.SCHEDULE.STATIC"(i32 0),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %I, i32 0, i32 1) ]

  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %I, align 4
  %5 = load i32, ptr %I, align 4
  %conv = sitofp i32 %5 to float
  %6 = load ptr, ptr %P.addr, align 8
  %7 = load i32, ptr %I, align 4
  %idxprom = sext i32 %7 to i64
  %arrayidx = getelementptr inbounds float, ptr %6, i64 %idxprom
  store float %conv, ptr %arrayidx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %8, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]

  ret void
}

define dso_local void @test4(ptr noundef %P) {
entry:
  %P.addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store ptr %P, ptr %P.addr, align 8
  store i32 99, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.SAFELEN"(i32 4),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %I, i32 0, i32 1, i32 1) ]

  store i32 0, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %1 = load i32, ptr %.omp.iv, align 4
  %2 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %1, %2
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %3 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %3, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %I, align 4
  %4 = load i32, ptr %I, align 4
  %conv = sitofp i32 %4 to float
  %5 = load ptr, ptr %P.addr, align 8
  %6 = load i32, ptr %I, align 4
  %idxprom = sext i32 %6 to i64
  %arrayidx = getelementptr inbounds float, ptr %5, i64 %idxprom
  store float %conv, ptr %arrayidx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %7, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond, !llvm.loop !5

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]

  ret void
}

!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.vectorize.enable", i1 true}
