; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -S %s | FileCheck %s

; Original code:
; void foo(int n) {
; #pragma omp simd collapse(2)
;   for (int i = 0; i < n; ++i)
;     for (int j = 0; j < n; ++j);
; }

; CHECK-NOT: QUAL.OMP.FIRSTPRIVATE:TYPED
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()
; CHECK-SAME: QUAL.OMP.COLLAPSE
; CHECK-SAME: QUAL.OMP.NORMALIZED.IV:TYPED
; CHECK-SAME: QUAL.OMP.NORMALIZED.UB:TYPED
; CHECK-SAME: QUAL.OMP.PRIVATE:TYPED

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local void @foo(i32 noundef %n) {
entry:
  %n.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %tmp1 = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.capture_expr.2 = alloca i64, align 8
  %.omp.uncollapsed.iv = alloca i64, align 8
  %.omp.uncollapsed.iv14 = alloca i64, align 8
  %.omp.uncollapsed.lb = alloca i64, align 8
  %.omp.uncollapsed.ub = alloca i64, align 8
  %.omp.uncollapsed.lb21 = alloca i64, align 8
  %.omp.uncollapsed.ub22 = alloca i64, align 8
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 %n, ptr %n.addr, align 4
  %0 = load i32, ptr %n.addr, align 4
  store i32 %0, ptr %.capture_expr.0, align 4
  %1 = load i32, ptr %n.addr, align 4
  store i32 %1, ptr %.capture_expr.1, align 4
  %2 = load i32, ptr %.capture_expr.0, align 4
  %sub = sub nsw i32 %2, 0
  %sub2 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub2, 1
  %div = sdiv i32 %add, 1
  %conv = sext i32 %div to i64
  %3 = load i32, ptr %.capture_expr.1, align 4
  %sub3 = sub nsw i32 %3, 0
  %sub4 = sub nsw i32 %sub3, 1
  %add5 = add nsw i32 %sub4, 1
  %div6 = sdiv i32 %add5, 1
  %conv7 = sext i32 %div6 to i64
  %mul = mul nsw i64 %conv, %conv7
  %sub8 = sub nsw i64 %mul, 1
  store i64 %sub8, ptr %.capture_expr.2, align 8
  %4 = load i32, ptr %.capture_expr.0, align 4
  %cmp = icmp slt i32 0, %4
  br i1 %cmp, label %land.lhs.true, label %omp.precond.end

land.lhs.true:                                    ; preds = %entry
  %5 = load i32, ptr %.capture_expr.1, align 4
  %cmp11 = icmp slt i32 0, %5
  br i1 %cmp11, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %land.lhs.true
  store i64 0, ptr %.omp.uncollapsed.lb, align 8
  %6 = load i32, ptr %.capture_expr.0, align 4
  %sub15 = sub nsw i32 %6, 0
  %sub16 = sub nsw i32 %sub15, 1
  %add17 = add nsw i32 %sub16, 1
  %div18 = sdiv i32 %add17, 1
  %conv19 = sext i32 %div18 to i64
  %sub20 = sub nsw i64 %conv19, 1
  store i64 %sub20, ptr %.omp.uncollapsed.ub, align 8
  store i64 0, ptr %.omp.uncollapsed.lb21, align 8
  %7 = load i32, ptr %.capture_expr.1, align 4
  %sub23 = sub nsw i32 %7, 0
  %sub24 = sub nsw i32 %sub23, 1
  %add25 = add nsw i32 %sub24, 1
  %div26 = sdiv i32 %add25, 1
  %conv27 = sext i32 %div26 to i64
  %sub28 = sub nsw i64 %conv27, 1
  store i64 %sub28, ptr %.omp.uncollapsed.ub22, align 8
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.COLLAPSE"(i32 2),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.uncollapsed.iv, i64 0, ptr %.omp.uncollapsed.iv14, i64 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.uncollapsed.ub, i64 0, ptr %.omp.uncollapsed.ub22, i64 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1) ]

  %9 = load i64, ptr %.omp.uncollapsed.lb, align 8
  store i64 %9, ptr %.omp.uncollapsed.iv, align 8
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc42, %omp.precond.then
  %10 = load i64, ptr %.omp.uncollapsed.iv, align 8
  %11 = load i64, ptr %.omp.uncollapsed.ub, align 8
  %cmp29 = icmp sle i64 %10, %11
  br i1 %cmp29, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end44

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %12 = load i64, ptr %.omp.uncollapsed.lb21, align 8
  store i64 %12, ptr %.omp.uncollapsed.iv14, align 8
  br label %omp.uncollapsed.loop.cond31

omp.uncollapsed.loop.cond31:                      ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body
  %13 = load i64, ptr %.omp.uncollapsed.iv14, align 8
  %14 = load i64, ptr %.omp.uncollapsed.ub22, align 8
  %cmp32 = icmp sle i64 %13, %14
  br i1 %cmp32, label %omp.uncollapsed.loop.body34, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body34:                      ; preds = %omp.uncollapsed.loop.cond31
  %15 = load i64, ptr %.omp.uncollapsed.iv, align 8
  %mul35 = mul nsw i64 %15, 1
  %add36 = add nsw i64 0, %mul35
  %conv37 = trunc i64 %add36 to i32
  store i32 %conv37, ptr %i, align 4
  %16 = load i64, ptr %.omp.uncollapsed.iv14, align 8
  %mul38 = mul nsw i64 %16, 1
  %add39 = add nsw i64 0, %mul38
  %conv40 = trunc i64 %add39 to i32
  store i32 %conv40, ptr %j, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.uncollapsed.loop.body34
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.body.continue
  %17 = load i64, ptr %.omp.uncollapsed.iv14, align 8
  %add41 = add nsw i64 %17, 1
  store i64 %add41, ptr %.omp.uncollapsed.iv14, align 8
  br label %omp.uncollapsed.loop.cond31

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond31
  br label %omp.uncollapsed.loop.inc42

omp.uncollapsed.loop.inc42:                       ; preds = %omp.uncollapsed.loop.end
  %18 = load i64, ptr %.omp.uncollapsed.iv, align 8
  %add43 = add nsw i64 %18, 1
  store i64 %add43, ptr %.omp.uncollapsed.iv, align 8
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end44:                       ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.SIMD"() ]

  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.uncollapsed.loop.end44, %land.lhs.true, %entry
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

