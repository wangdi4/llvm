; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s --check-prefix=CRITICAL --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s --check-prefix=CRITICAL --check-prefix=ALL
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL

; Test src:
;
; #include <xmmintrin.h>
; #define N 1000
;
; void reduce(const __m128* vals, __m128 *sum) {
; #pragma omp parallel for reduction(+:sum[0:N])
;   for (int i = 0; i < N; ++i)
;     sum[i] += vals[i];
; }

; The test IR was hand-modified to use a constant section size/offset for
; reduction. CFE currently generates IR instructions to compute them.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @reduce(ptr %vals, ptr %sum) #0 {
entry:
  %vals.addr = alloca ptr, align 8
  %sum.addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr %vals, ptr %vals.addr, align 8
  store ptr %sum, ptr %sum.addr, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 999, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.PTR_TO_PTR.TYPED"(ptr %sum.addr, <4 x float> zeroinitializer, i64 1000, i64 0),
    "QUAL.OMP.SHARED:TYPED"(ptr %vals.addr, ptr null, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

; ALL-NOT: "QUAL.OMP.REDUCTION.ADD:ARRSECT"
; FASTRED: define internal void @[[REDUCE_CALLBACK:[^,]+]](ptr %dst, ptr %src) {
; CRITICAL: %sum.addr.red = alloca [1000 x <4 x float>], align 8
; CRITICAL: store <4 x float> zeroinitializer, ptr %red.cpy.dest.ptr, align 16
; CRITICAL: call void @__kmpc_critical(ptr @.kmpc_loc{{.*}}, i32 %my.tid17{{.*}}, ptr @{{.*}})
; ALL: red.update.body:
; ALL: %[[LOCAL_VAL:[^,]+]] = load <4 x float>, ptr %[[LOCAL:[^,]+]], align 16
; ALL-NEXT: %[[GLOBAL_VAL:[^,]+]] = load <4 x float>, ptr %[[GLOBAL:[^,]+]], align 16
; ALL-NEXT: %[[SUM:[^,]+]] = fadd <4 x float> %[[GLOBAL_VAL]], %[[LOCAL_VAL]]
; ALL-NEXT: store <4 x float> %[[SUM]], ptr %[[GLOBAL]], align 16
; CRITICAL: call void @__kmpc_end_critical(ptr @.kmpc_loc{{.*}}, i32 %my.tid{{.*}}, ptr @{{.*}})
; FASTRED:  %[[RET:[^,]+]] = call i32 @__kmpc_reduce(ptr @.kmpc_loc{{.*}}, i32 %my.tid{{.*}}, i32 1, i32 16000, ptr %{{.*}}, ptr @[[REDUCE_CALLBACK]], ptr @{{.*}})
; FASTRED-NEXT: %[[TO:[^,]+]] = icmp eq i32 %[[RET]], 1
; FASTRED-NEXT: br i1 %[[TO]], label %tree.reduce, label %tree.reduce.exit
; FASTRED: tree.reduce:
; FASTRED: tree.reduce.exit:
; FASTRED: red.update.body:
; FASTRED: %[[RED_LOCAL_VAL:[^,]+]] = load <4 x float>, ptr %[[RED_LOCAL:[^,]+]], align 16
; FASTRED-NEXT: %[[RED_GLOBAL_VAL:[^,]+]] = load <4 x float>, ptr %[[RED_GLOBAL:[^,]+]], align 16
; FASTRED-NEXT: %[[RED_SUM:[^,]+]] = fadd <4 x float> %[[RED_GLOBAL_VAL]], %[[RED_LOCAL_VAL]]
; FASTRED-NEXT: store <4 x float> %[[RED_SUM]], ptr %[[RED_GLOBAL]], align 16
; FASTRED: call void @__kmpc_end_reduce(ptr @.kmpc_loc{{.*}}, i32 %my.tid{{.*}}, ptr @{{.*}})
; FASTRED-NEXT: br label %tree.reduce.exit

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
  %8 = load ptr, ptr %sum.addr, align 8
  %9 = load i32, ptr %i, align 4
  %idxprom1 = sext i32 %9 to i64
  %arrayidx2 = getelementptr inbounds <4 x float>, ptr %8, i64 %idxprom1
  %10 = load <4 x float>, ptr %arrayidx2, align 16
  %add3 = fadd <4 x float> %10, %7
  store <4 x float> %add3, ptr %arrayidx2, align 16
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %11 = load i32, ptr %.omp.iv, align 4
  %add4 = add nsw i32 %11, 1
  store i32 %add4, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
