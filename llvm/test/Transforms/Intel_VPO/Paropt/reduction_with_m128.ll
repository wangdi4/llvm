; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
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


; ModuleID = 'reduction_with_m128.c'
source_filename = "reduction_with_m128.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local <4 x float> @reduce(<4 x float>* %vals) #0 {
entry:
  %vals.addr = alloca <4 x float>*, align 8
  %sum = alloca <4 x float>, align 16
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store <4 x float>* %vals, <4 x float>** %vals.addr, align 8
  %call = call <4 x float> @_mm_setzero_ps()
  store <4 x float> %call, <4 x float>* %sum, align 16
  store i32 0, i32* %.omp.lb, align 4
  store i32 999, i32* %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD"(<4 x float>* %sum), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"(<4 x float>** %vals.addr) ]

; CHECK-NOT: "QUAL.OMP.REDUCTION.ADD"
; CHECK-NOT: __kmpc_atomic
; CHECK: %sum.red = alloca <4 x float>, align 16
; CHECK: store <4 x float> zeroinitializer, <4 x float>* %sum.red, align 16
; CHECK: %[[LOCAL_VAL:[^,]+]] = load <4 x float>, <4 x float>* %[[LOCAL:[^,]+]], align 16
; CHECK-NEXT: %[[GLOBAL_VAL:[^,]+]] = load <4 x float>, <4 x float>* %[[GLOBAL:[^,]+]], align 16
; CHECK-NEXT: %[[SUM:[^,]+]] = fadd <4 x float> %[[GLOBAL_VAL]], %[[LOCAL_VAL]]
; CHECK-NEXT: store <4 x float> %[[SUM]], <4 x float>* %[[GLOBAL]], align 16

  %1 = load i32, i32* %.omp.lb, align 4
  store i32 %1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, i32* %.omp.iv, align 4
  %3 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %5 = load <4 x float>*, <4 x float>** %vals.addr, align 8
  %6 = load i32, i32* %i, align 4
  %idxprom = sext i32 %6 to i64
  %arrayidx = getelementptr inbounds <4 x float>, <4 x float>* %5, i64 %idxprom
  %7 = load <4 x float>, <4 x float>* %arrayidx, align 16
  %8 = load <4 x float>, <4 x float>* %sum, align 16
  %add1 = fadd <4 x float> %8, %7
  store <4 x float> %add1, <4 x float>* %sum, align 16
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, i32* %.omp.iv, align 4
  %add2 = add nsw i32 %9, 1
  store i32 %add2, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %10 = load <4 x float>, <4 x float>* %sum, align 16
  ret <4 x float> %10
}

; Function Attrs: alwaysinline nounwind uwtable
define internal <4 x float> @_mm_setzero_ps() #1 {
entry:
  %.compoundliteral = alloca <4 x float>, align 16
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral, align 16
  %0 = load <4 x float>, <4 x float>* %.compoundliteral, align 16
  ret <4 x float> %0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="128" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { alwaysinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="128" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
