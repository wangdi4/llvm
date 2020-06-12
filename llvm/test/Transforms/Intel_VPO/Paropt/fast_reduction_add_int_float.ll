; RUN: opt < %s -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S | FileCheck %s --check-prefix=USE-LOCAL --check-prefix=ALL
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt'  -S | FileCheck %s --check-prefix=USE-LOCAL --check-prefix=ALL
; RUN: opt < %s -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction-ctrl=0 -S | FileCheck %s --check-prefix=USE-REC --check-prefix=ALL
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction-ctrl=0  -S | FileCheck %s --check-prefix=USE-REC --check-prefix=ALL


;
; int main(void)
; {
;   int i, sum = 0;
;   double f = 0.0;
;
; #pragma omp parallel for reduction(+:sum) reduction(+:f)
;   for (i=0; i<10; i++) {
;     sum+=i;
;     f += (float)i;
;   }
;
;   return 0;
; }


; ModuleID = 'fast_reduction_add_int_float.c'
source_filename = "fast_reduction_add_int_float.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %sum = alloca i32, align 4
  %f = alloca double, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 0, i32* %sum, align 4
  store double 0.000000e+00, double* %f, align 8
  store i32 0, i32* %.omp.lb, align 4
  store i32 9, i32* %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD"(i32* %sum), "QUAL.OMP.REDUCTION.ADD"(double* %f), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]

; ALL-NOT: "QUAL.OMP.REDUCTION.ADD"
; ALL: %struct.fast_red_t = type <{ i32, double }>
; ALL: define internal void @main_tree_reduce_{{.*}}(i8* %dst, i8* %src) {
; ALL: declare i32 @__kmpc_reduce(%struct.ident_t*, i32, i32, i32, i8*, void (i8*, i8*)*, [8 x i32]*)
; ALL: declare void @__kmpc_end_reduce(%struct.ident_t*, i32, [8 x i32]*)
; ALL: %fast_red_struct{{.*}} = alloca %struct.fast_red_t, align 8
; USE-LOCAL: %[[I32_LOCAL:[^,]+]] = alloca i32, align 4
; USE-LOCAL-NEXT: %[[FP_LOCAL:[^,]+]] = alloca double, align 8
; USE-REC: %[[I32_REC:[^,]+]] = getelementptr inbounds %struct.fast_red_t, %struct.fast_red_t* %fast_red_struct, i32 0, i32 0
; USE-REC-NEXT: %[[FP_REC:[^,]+]] = getelementptr inbounds %struct.fast_red_t, %struct.fast_red_t* %fast_red_struct, i32 0, i32 1

; USE-LOCAL: %[[FP_REC:[^,]+]] = getelementptr inbounds %struct.fast_red_t, %struct.fast_red_t* %fast_red_struct, i32 0, i32 1
; CHECK-USE-LOCAL-NEXT: store double 0.000000e+00, double* %[[FP_REC]], align 8
; USE-LOCAL: store double 0.000000e+00, double* %[[FP_LOCAL]], align 8
; USE-LOCAL: %[[I32_REC:[^,]+]] = getelementptr inbounds %struct.fast_red_t, %struct.fast_red_t* %fast_red_struct, i32 0, i32 0
; CHECK-USE-LOCAL-NEXT: store i32 0, i32* %[[I32_REC]], align 4
; USE-LOCAL: store i32 0, i32* %[[I32_LOCAL]], align 4
; USE-LOCAL: %[[I32_LOCAL_VAL:[^,]+]] = load i32, i32* %[[I32_LOCAL]], align 4
; USE-LOCAL-NEXT: %[[I32_ADD:[^,]+]] = add nsw i32 %[[I32_LOCAL_VAL]], %{{.*}}
; USE-LOCAL-NEXT: store i32 %[[I32_ADD]], i32* %[[I32_LOCAL]], align 4
; USE-LOCAL: %[[FP_LOCAL_VAL:[^,]+]] = load double, double* %[[FP_LOCAL]], align 8
; USE-LOCAL-NEXT: %[[FP_ADD:[^,]+]] = fadd double %[[FP_LOCAL_VAL]], %{{.*}}
; USE-LOCAL-NEXT: store double %[[FP_ADD]], double* %[[FP_LOCAL]], align 8

; USE-REC: store double 0.000000e+00, double* %[[FP_REC]], align 8
; USE-REC: store i32 0, i32* %[[I32_REC]], align 4
; USE-REC: %[[I32_REC_VAL:[^,]+]] = load i32, i32* %[[I32_REC]], align 4
; USE-REC-NEXT: %[[I32_ADD:[^,]+]] = add nsw i32 %[[I32_REC_VAL]], %{{.*}}
; USE-REC-NEXT: store i32 %[[I32_ADD]], i32* %[[I32_REC]], align 4
; USE-REC: %[[FP_REC_VAL:[^,]+]] = load double, double* %[[FP_REC]], align 8
; USE-REC-NEXT: %[[FP_ADD:[^,]+]] = fadd double %[[FP_REC_VAL]], %{{.*}}
; USE-REC-NEXT: store double %[[FP_ADD]], double* %[[FP_REC]], align 8

; USE-LOCAL: %[[I32_LOCAL_VAL2:[^,]+]] = load i32, i32* %[[I32_LOCAL]], align 4
; USE-LOCAL-NEXT: store i32 %[[I32_LOCAL_VAL2]], i32* %[[I32_REC]]
; USE-LOCAL: %[[FP_LOCAL_VAL2:[^,]+]] = load double, double* %[[FP_LOCAL]], align 8
; USE-LOCAL-NEXT: store double %[[FP_LOCAL_VAL2]], double* %[[FP_REC]], align 8
; USE-LOCAL: call void @__kmpc_for_static_fini(%struct.ident_t* @{{.*}}, i32 %{{.*}})

; ALL: %[[BITCAST:[^,]+]] = bitcast %struct.fast_red_t* %fast_red_struct to i8*
; ALL: %[[RET:[^,]+]] = call i32 @__kmpc_reduce(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 2, i32 12, i8* %[[BITCAST]], void (i8*, i8*)* @main_tree_reduce_{{.*}}, [8 x i32]* @{{.*}})
; ALL-NEXT: %to.tree.reduce = icmp eq i32 %[[RET]], 1
; ALL-NEXT: br i1 %to.tree.reduce, label %tree.reduce, label %tree.reduce.exit
; ALL: tree.reduce:
; ALL: tree.reduce.exit:
; ALL-NEXT: {{.*}} = phi i1 [ false, {{.*}} ], [ true, {{.*}} ]
; ALL: %[[COMB_I32_LOCAL_VAL:[^,]+]] = load i32, i32* %[[I32_REC]], align 4
; ALL-NEXT: %[[COMB_I32_GLOBAL_VAL:[^,]+]] = load i32, i32* %[[COMB_I32_GLOBAL:[^,]+]], align 4
; ALL-NEXT: %[[COMB_I32_ADD:[^,]+]] = add i32 %[[COMB_I32_GLOBAL_VAL]], %[[COMB_I32_LOCAL_VAL]]
; ALL-NEXT: store i32 %[[COMB_I32_ADD]], i32* %[[COMB_I32_GLOBAL]], align 4
; ALL: %[[COMB_FP_LOCAL_VAL:[^,]+]] = load double, double* %[[FP_REC]], align 8
; ALL-NEXT: %[[COMB_FP_GLOBAL_VAL:[^,]+]] = load double, double* %[[COMB_FP_GLOBAL:[^,]+]], align 8
; ALL-NEXT: %[[COMB_FP_ADD:[^,]+]] = fadd double %[[COMB_FP_GLOBAL_VAL]], %[[COMB_FP_LOCAL_VAL]]
; ALL-NEXT: store double %[[COMB_FP_ADD]], double* %[[COMB_FP_GLOBAL]], align 8
; ALL: call void @__kmpc_end_reduce({{.*}})
; ALL-NEXT: br label %tree.reduce.exit


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
  %5 = load i32, i32* %i, align 4
  %6 = load i32, i32* %sum, align 4
  %add1 = add nsw i32 %6, %5
  store i32 %add1, i32* %sum, align 4
  %7 = load i32, i32* %i, align 4
  %conv = sitofp i32 %7 to float
  %conv2 = fpext float %conv to double
  %8 = load double, double* %f, align 8
  %add3 = fadd double %8, %conv2
  store double %add3, double* %f, align 8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, i32* %.omp.iv, align 4
  %add4 = add nsw i32 %9, 1
  store i32 %add4, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
