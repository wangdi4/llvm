; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s
;
; Test src:
;
; void byref_reduction_array(int (&y_Arr_ref)[3][4][5]) {
;   #pragma omp parallel for reduction(+:y_Arr_ref)
;   for (int i = 0; i < 3; i++) {
;     for (int j = 0; j < 4; j++) {
;       for (int k = 0; k < 5; k++) {
;         y_Arr_ref[1][2][k] += 1;
;       }
;     }
;   }
; }
;
; ModuleID = 'byref_reduction_array.cpp'
source_filename = "byref_reduction_array.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z21byref_reduction_arrayRA3_A4_A5_i([3 x [4 x [5 x i32]]]* dereferenceable(240) %y_Arr_ref) #0 {
entry:
  %y_Arr_ref.addr = alloca [3 x [4 x [5 x i32]]]*, align 8
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %k = alloca i32, align 4
  store [3 x [4 x [5 x i32]]]* %y_Arr_ref, [3 x [4 x [5 x i32]]]** %y_Arr_ref.addr, align 8
  store i32 0, i32* %.omp.lb, align 4
  store i32 2, i32* %.omp.ub, align 4
  store i32 1, i32* %.omp.stride, align 4
  store i32 0, i32* %.omp.is_last, align 4
  %0 = load [3 x [4 x [5 x i32]]]*, [3 x [4 x [5 x i32]]]** %y_Arr_ref.addr, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD:BYREF"([3 x [4 x [5 x i32]]]** %y_Arr_ref.addr), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %k) ]
; CHECK-NOT: call token @llvm.directive.region.entry()
; CHECK-NOT: call token @llvm.directive.region.exit()

; Zero-trip test for reduction array initialization
; CHECK-DAG: %[[LOCAL_END:[a-zA-Z._0-9]+]] = getelementptr i32, i32* %[[LOCAL_BEGIN:[a-zA-Z._0-9]+]], i32 60
; CHECK-DAG: %[[LOCAL_BEGIN]] = getelementptr inbounds [3 x [4 x [5 x i32]]], [3 x [4 x [5 x i32]]]* %[[LOCAL:[a-zA-Z._0-9]+]], i32 0, i32 0, i32 0, i32 0
; CHECK-DAG: %{{[a-zA-Z._0-9]+}} = icmp eq i32* %[[LOCAL_BEGIN]], %[[LOCAL_END]]

; Save local array's address to a var which will replace uses of original var within the region.
; CHECK-DAG: store [3 x [4 x [5 x i32]]]* %[[LOCAL]], [3 x [4 x [5 x i32]]]** %[[LOCAL_REF:[a-zA-Z._0-9]+]]

  %2 = load i32, i32* %.omp.lb, align 4
  store i32 %2, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, i32* %.omp.iv, align 4
  %4 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  store i32 0, i32* %j, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc8, %omp.inner.for.body
  %6 = load i32, i32* %j, align 4
  %cmp1 = icmp slt i32 %6, 4
  br i1 %cmp1, label %for.body, label %for.end10

for.body:                                         ; preds = %for.cond
  store i32 0, i32* %k, align 4
  br label %for.cond2

for.cond2:                                        ; preds = %for.inc, %for.body
  %7 = load i32, i32* %k, align 4
  %cmp3 = icmp slt i32 %7, 5
  br i1 %cmp3, label %for.body4, label %for.end

for.body4:                                        ; preds = %for.cond2
; Check for the replacement of original %y_Arr_ref.addr with LOCAL_REF.
; CHECK-DAG: %{{[a-zA-Z._0-9]+}} = load [3 x [4 x [5 x i32]]]*, [3 x [4 x [5 x i32]]]** %[[LOCAL_REF]]
  %8 = load [3 x [4 x [5 x i32]]]*, [3 x [4 x [5 x i32]]]** %y_Arr_ref.addr, align 8
  %arrayidx = getelementptr inbounds [3 x [4 x [5 x i32]]], [3 x [4 x [5 x i32]]]* %8, i64 0, i64 1
  %arrayidx5 = getelementptr inbounds [4 x [5 x i32]], [4 x [5 x i32]]* %arrayidx, i64 0, i64 2
  %9 = load i32, i32* %k, align 4
  %idxprom = sext i32 %9 to i64
  %arrayidx6 = getelementptr inbounds [5 x i32], [5 x i32]* %arrayidx5, i64 0, i64 %idxprom
  %10 = load i32, i32* %arrayidx6, align 4
  %add7 = add nsw i32 %10, 1
  store i32 %add7, i32* %arrayidx6, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body4
  %11 = load i32, i32* %k, align 4
  %inc = add nsw i32 %11, 1
  store i32 %inc, i32* %k, align 4
  br label %for.cond2

for.end:                                          ; preds = %for.cond2
  br label %for.inc8

for.inc8:                                         ; preds = %for.end
  %12 = load i32, i32* %j, align 4
  %inc9 = add nsw i32 %12, 1
  store i32 %inc9, i32* %j, align 4
  br label %for.cond

for.end10:                                        ; preds = %for.cond
  br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end10
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %13 = load i32, i32* %.omp.iv, align 4
  %add11 = add nsw i32 %13, 1
  store i32 %add11, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0"}
