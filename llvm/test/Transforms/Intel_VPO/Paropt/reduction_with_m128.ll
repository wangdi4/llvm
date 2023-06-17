; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s


; #include <xmmintrin.h>
;
; __m128 reduce(const __m128* vals) {
;   __m128 sum = _mm_setzero_ps();
; #pragma omp parallel for reduction(+:sum)
;   for (int i = 0; i < 1000; ++i)
;     sum += vals[i];
;   return sum;
; }

; CHECK-NOT: "QUAL.OMP.REDUCTION.ADD"
; CHECK-NOT: __kmpc_atomic
; CHECK: %sum.red = alloca <4 x float>, align 16
; CHECK: store <4 x float> zeroinitializer, ptr %sum.red, align 16
; CHECK: %[[LOCAL_VAL:[^,]+]] = load <4 x float>, ptr %[[LOCAL:[^,]+]], align 16
; CHECK-NEXT: %[[GLOBAL_VAL:[^,]+]] = load <4 x float>, ptr %[[GLOBAL:[^,]+]], align 16
; CHECK-NEXT: %[[SUM:[^,]+]] = fadd fast <4 x float> %[[GLOBAL_VAL]], %[[LOCAL_VAL]]
; CHECK-NEXT: store <4 x float> %[[SUM]], ptr %[[GLOBAL]], align 16

; ModuleID = 'reduction_with_m128.c'
source_filename = "reduction_with_m128.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local nofpclass(nan inf) <4 x float> @reduce(ptr noundef %vals) {
entry:
  %vals.addr = alloca ptr, align 8
  %sum = alloca <4 x float>, align 16
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr %vals, ptr %vals.addr, align 8
  %call = call fast nofpclass(nan inf) <4 x float> @_mm_setzero_ps()
  store <4 x float> %call, ptr %sum, align 16
  store i32 0, ptr %.omp.lb, align 4
  store i32 999, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %sum, <4 x float> zeroinitializer, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %vals.addr, ptr null, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

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
  store i32 %add, ptr %i, align 4
  %5 = load ptr, ptr %vals.addr, align 8
  %6 = load i32, ptr %i, align 4
  %idxprom = sext i32 %6 to i64
  %arrayidx = getelementptr inbounds <4 x float>, ptr %5, i64 %idxprom
  %7 = load <4 x float>, ptr %arrayidx, align 16
  %8 = load <4 x float>, ptr %sum, align 16
  %add1 = fadd fast <4 x float> %8, %7
  store <4 x float> %add1, ptr %sum, align 16
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, ptr %.omp.iv, align 4
  %add2 = add nsw i32 %9, 1
  store i32 %add2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  %10 = load <4 x float>, ptr %sum, align 16
  ret <4 x float> %10
}

define internal nofpclass(nan inf) <4 x float> @_mm_setzero_ps() {
entry:
  %.compoundliteral = alloca <4 x float>, align 16
  store <4 x float> zeroinitializer, ptr %.compoundliteral, align 16
  %0 = load <4 x float>, ptr %.compoundliteral, align 16
  ret <4 x float> %0
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
