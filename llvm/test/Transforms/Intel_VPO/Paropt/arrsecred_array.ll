; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-cfg-restructuring -vpo-paropt -S < %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-cfg-restructuring),vpo-paropt' -S < %s  | FileCheck %s
;
; Test src:
;
; static int y[3][4][5];
;
; void arrsecred_array() {
;   #pragma omp parallel for reduction(+ : y [1:2][:][:])
;   for (int i = 1; i < 3; i++) {
;     for (int j = 0; j < 4; j++) {
;       for (int k = 0; k < 5; k++) {
;         y[1][2][k] += 1;
;       }
;     }
;   }
; }
;
; ModuleID = 'arrsecred_array.cpp'
source_filename = "arrsecred_array.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@_Z1y = internal global [3 x [4 x [5 x i32]]] zeroinitializer, align 16

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z15arrsecred_arrayv() #0 {
entry:
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %k = alloca i32, align 4
  store i32 0, i32* %.omp.lb, align 4
  store i32 1, i32* %.omp.ub, align 4
  store i32 1, i32* %.omp.stride, align 4
  store i32 0, i32* %.omp.is_last, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD:ARRSECT"([3 x [4 x [5 x i32]]]* @_Z1y, i64 3, i64 1, i64 2, i64 1, i64 0, i64 4, i64 1, i64 0, i64 5, i64 1), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %k) ]
; CHECK-NOT: call token @llvm.directive.region.entry()
; CHECK-NOT: call token @llvm.directive.region.exit()
; Check for array section reduction local copy preparation
; CHECK-DAG: %[[LOCAL_MINUS_OFFSET:[a-zA-Z._0-9]+]] = getelementptr i32, i32* %[[LOCAL:[a-zA-Z._0-9]+]], i64 -20
; Zero-trip test for reduction array initialization
; CHECK-DAG: %[[LOCAL_END:[0-9]+]] = getelementptr i32, i32* %[[LOCAL]], i64 40
; CHECK-DAG: %{{[a-zA-Z._0-9]+}} = icmp eq i32* %[[LOCAL]], %[[LOCAL_END]]
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
  %add = add nsw i32 1, %mul
  store i32 %add, i32* %i, align 4
  store i32 0, i32* %j, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc6, %omp.inner.for.body
  %5 = load i32, i32* %j, align 4
  %cmp1 = icmp slt i32 %5, 4
  br i1 %cmp1, label %for.body, label %for.end8

for.body:                                         ; preds = %for.cond
  store i32 0, i32* %k, align 4
  br label %for.cond2

for.cond2:                                        ; preds = %for.inc, %for.body
  %6 = load i32, i32* %k, align 4
  %cmp3 = icmp slt i32 %6, 5
  br i1 %cmp3, label %for.body4, label %for.end

for.body4:                                        ; preds = %for.cond2
  %7 = load i32, i32* %k, align 4
  %idxprom = sext i32 %7 to i64
; Check for the replacement of original @_Z1y with "%y_local - offset".
; Bitcast is from the code-motion fence introduced by the prepare pass.
; CHECK-DAG: %{{[0-9]+}} = bitcast i32* %[[LOCAL_MINUS_OFFSET]] to i8*
  %arrayidx = getelementptr inbounds [5 x i32], [5 x i32]* getelementptr inbounds ([3 x [4 x [5 x i32]]], [3 x [4 x [5 x i32]]]* @_Z1y, i64 0, i64 1, i64 2), i64 0, i64 %idxprom
  %8 = load i32, i32* %arrayidx, align 4
  %add5 = add nsw i32 %8, 1
  store i32 %add5, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body4
  %9 = load i32, i32* %k, align 4
  %inc = add nsw i32 %9, 1
  store i32 %inc, i32* %k, align 4
  br label %for.cond2

for.end:                                          ; preds = %for.cond2
  br label %for.inc6

for.inc6:                                         ; preds = %for.end
  %10 = load i32, i32* %j, align 4
  %inc7 = add nsw i32 %10, 1
  store i32 %inc7, i32* %j, align 4
  br label %for.cond

for.end8:                                         ; preds = %for.cond
  br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end8
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %11 = load i32, i32* %.omp.iv, align 4
  %add9 = add nsw i32 %11, 1
  store i32 %add9, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

  ; Zero-trip test for reduction array finalization
; CHECK-DAG: icmp eq (i32* getelementptr (i32, i32* getelementptr inbounds ([3 x [4 x [5 x i32]]], [3 x [4 x [5 x i32]]]* @_Z1y, i32 0, i32 0, i32 0, i32 0), i64 20), i32* getelementptr (i32, i32* getelementptr inbounds ([3 x [4 x [5 x i32]]], [3 x [4 x [5 x i32]]]* @_Z1y, i32 0, i32 0, i32 0, i32 0), i64 60))

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0"}
