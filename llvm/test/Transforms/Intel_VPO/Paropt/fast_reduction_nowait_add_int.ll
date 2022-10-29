; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s --check-prefix=USE-LOCAL --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s --check-prefix=USE-LOCAL --check-prefix=ALL
; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction-ctrl=0 -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s --check-prefix=USE-REC --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction-ctrl=0 -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s --check-prefix=USE-REC --check-prefix=ALL

; Test src:
;
; int main(void) {
;   int i, sum = 0;
; #pragma omp parallel
;   {
; #pragma omp for reduction(+ : sum) nowait
;     for (i = 0; i < 10; i++) {
;       sum += i;
;     }
;   }
;   return 0;
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %sum = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  store i32 0, ptr %sum, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %sum, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]
  store i32 0, ptr %.omp.lb, align 4
  store i32 9, ptr %.omp.ub, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %sum, i32 0, i32 1),
    "QUAL.OMP.NOWAIT"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0) ]

; ALL-NOT: "QUAL.OMP.REDUCTION.ADD"
; ALL: %struct.fast_red_t = type <{ i32 }>
; ALL: define internal void @main_tree_reduce_{{.*}}(ptr %dst, ptr %src) {
; ALL-NEXT: entry:
; ALL-NEXT:   %dst.sum = getelementptr inbounds %struct.fast_red_t, ptr %dst, i32 0, i32 0
; ALL-NEXT:   %src.sum = getelementptr inbounds %struct.fast_red_t, ptr %src, i32 0, i32 0
; ALL-NEXT:   %0 = load i32, ptr %src.sum, align 4
; ALL-NEXT:   %1 = load i32, ptr %dst.sum, align 4
; ALL-NEXT:   %2 = add i32 %1, %0
; ALL-NEXT:   store i32 %2, ptr %dst.sum, align 4
; ALL-NEXT:   ret void
; ALL-NEXT: }
; ALL: declare i32 @__kmpc_reduce_nowait(ptr, i32, i32, i32, ptr, ptr, ptr)
; ALL: declare void @__kmpc_end_reduce_nowait(ptr, i32, ptr)

; USE-LOCAL: %[[LOCAL:[^,]+.red]] = alloca i32, align 4
; ALL: %fast_red_struct{{.*}} = alloca %struct.fast_red_t, align 8
; USE-REC: %[[REC:[^,]+]] = getelementptr inbounds %struct.fast_red_t, ptr %fast_red_struct, i32 0, i32 0
; USE-REC: store i32 0, ptr %[[REC]]

; USE-LOCAL: %[[REC:[^,]+]] = getelementptr inbounds %struct.fast_red_t, ptr %fast_red_struct, i32 0, i32 0
; USE-LOCAL: store i32 0, ptr %[[LOCAL]]

; USE-LOCAL: %[[LOCAL_VAL:[^,]+]] = load i32, ptr %[[LOCAL]], align 4
; USE-LOCAL-NEXT: %[[ADD:[^,]+]] = add nsw i32 %[[LOCAL_VAL]], %{{.*}}
; USE-LOCAL-NEXT: store i32 %[[ADD]], ptr %[[LOCAL]], align 4

; USE-REC: %[[REC_VAL:[^,]+]] = load i32, ptr %[[REC]], align 4
; USE-REC-NEXT: %[[ADD:[^,]+]] = add nsw i32 %[[REC_VAL]], %{{.*}}
; USE-REC-NEXT: store i32 %[[ADD]], ptr %[[REC]], align 4

; USE-LOCAL: %[[LOCAL_VAL2:[^,]+]] = load i32, ptr %[[LOCAL]], align 4
; USE-LOCAL-NEXT: store i32 %[[LOCAL_VAL2]], ptr %[[REC]]
; USE-LOCAL: call void @__kmpc_for_static_fini(ptr @{{.*}}, i32 %{{.*}})
; ALL: %[[RET:[^,]+]] = call i32 @__kmpc_reduce_nowait(ptr @{{.*}}, i32 %{{.*}}, i32 1, i32 4, ptr %fast_red_struct, ptr @main_tree_reduce_{{.*}}, ptr @{{.*}})
; ALL-NEXT: %to.tree.reduce = icmp eq i32 %[[RET]], 1
; ALL-NEXT: br i1 %to.tree.reduce, label %tree.reduce, label %tree.reduce.exit
; ALL: tree.reduce:
; ALL: tree.reduce.exit:
; ALL-NEXT: %[[PHI:[^,]+]] = phi i1 [ false, {{.*}} ], [ true, {{.*}} ]
; ALL-NEXT: %[[CMP_RESULT:[^,]+]] = icmp eq i1 %[[PHI]], false
; ALL-NEXT: br i1 %[[CMP_RESULT]], label %{{.*}}, label %[[ATOMIC_REDUCE_EXIT:[^,]+]]
; ALL: %[[COMB_LOCAL_VAL:[^,]+]] = load i32, ptr %[[REC]], align 4
; ALL-NEXT: %[[COMB_GLOBAL_VAL:[^,]+]] = load i32, ptr %[[COMB_GLOBAL:[^,]+]], align 4
; ALL-NEXT: %[[COMB_ADD:[^,]+]] = add i32 %[[COMB_GLOBAL_VAL]], %[[COMB_LOCAL_VAL]]
; ALL-NEXT: store i32 %[[COMB_ADD]], ptr %[[COMB_GLOBAL]], align 4
; ALL: %[[TO_ATOMIC_REDUCE:[^,]+]] = icmp eq i32 %[[RET]], 2
; ALL-NEXT: br i1 %[[TO_ATOMIC_REDUCE]], label %[[ATOMIC_REDUCE:[^,]+]], label %[[ATOMIC_REDUCE_EXIT]]
; ALL: [[ATOMIC_REDUCE_EXIT]]:
; ALL: call void @__kmpc_end_reduce_nowait({{.*}})
; ALL-NEXT: br label %tree.reduce.exit
; ALL: [[ATOMIC_REDUCE]]:
; ALL: %[[REC_LOCAL:[^,]+]] = load i32, ptr %[[REC]], align 4
; ALL: call void @__kmpc_atomic_fixed4_add(ptr @.kmpc_loc{{.*}}, i32 %my.tid{{.*}}, ptr %[[COMB_GLOBAL]], i32 %[[REC_LOCAL]])
; ALL-NOT: call void @__kmpc_end_reduce_nowait({{.*}})

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
  %6 = load i32, ptr %i, align 4
  %7 = load i32, ptr %sum, align 4
  %add1 = add nsw i32 %7, %6
  store i32 %add1, ptr %sum, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr %.omp.iv, align 4
  %add2 = add nsw i32 %8, 1
  store i32 %add2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.LOOP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
