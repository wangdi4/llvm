; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-cfg-restructuring -vpo-paropt -S < %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-cfg-restructuring),vpo-paropt' -S < %s  | FileCheck %s
;
; Test src:
;
; static int (*yarrayptr)[3][4][5];
;
; void arrsecred_arrayptr() {
;   #pragma omp parallel for reduction(+:yarrayptr[3][1][2:2][1:3])
;   for (int i = 0; i < 3; i++) {
;     for (int j = 2; j < 4; j++) {
;       for (int k = 1; k < 4; k++) {
;         yarrayptr[3][1][j][k] += 1;
;       }
;     }
;   }
; }
;
; ModuleID = 'arrsecred_arrayptr.cpp'
source_filename = "arrsecred_arrayptr.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@_Z9yarrayptr = internal global [3 x [4 x [5 x i32]]]* null, align 8

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z18arrsecred_arrayptrv() #0 {
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
  store i32 2, i32* %.omp.ub, align 4
  store i32 1, i32* %.omp.stride, align 4
  store i32 0, i32* %.omp.is_last, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD:ARRSECT"([3 x [4 x [5 x i32]]]** @_Z9yarrayptr, i64 4, i64 3, i64 1, i64 1, i64 1, i64 1, i64 1, i64 2, i64 2, i64 1, i64 1, i64 3, i64 1), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %k) ]
; CHECK-NOT: call token @llvm.directive.region.entry()
; CHECK-NOT: call token @llvm.directive.region.exit()
; Check for array section reduction local copy preparation
; CHECK-DAG: %[[LOCAL_MINUS_OFFSET:[a-zA-Z._0-9]+]] = getelementptr i32, i32* %[[LOCAL:[a-zA-Z._0-9]+]], i64 -211
; CHECK-DAG: store i32* %[[LOCAL_MINUS_OFFSET]], i32** %[[LOCAL_MINUS_OFFSET_ADDR:[a-zA-Z._0-9]+]]
; Zero-trip test for reduction array initialization
; CHECK-DAG: %[[LOCAL_END:[0-9]+]] = getelementptr i32, i32* %[[LOCAL]], i64 6
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
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  store i32 2, i32* %j, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc10, %omp.inner.for.body
  %5 = load i32, i32* %j, align 4
  %cmp1 = icmp slt i32 %5, 4
  br i1 %cmp1, label %for.body, label %for.end12

for.body:                                         ; preds = %for.cond
  store i32 1, i32* %k, align 4
  br label %for.cond2

for.cond2:                                        ; preds = %for.inc, %for.body
  %6 = load i32, i32* %k, align 4
  %cmp3 = icmp slt i32 %6, 4
  br i1 %cmp3, label %for.body4, label %for.end

for.body4:                                        ; preds = %for.cond2
  %7 = load [3 x [4 x [5 x i32]]]*, [3 x [4 x [5 x i32]]]** @_Z9yarrayptr, align 8
; Check for the replacement of original @_Z9yarrayptr with "&(y_local - offset)"
; CHECK-DAG: %{{[0-9]+}} = load [3 x [4 x [5 x i32]]]*, i32** %[[LOCAL_MINUS_OFFSET_ADDR]]
  %arrayidx = getelementptr inbounds [3 x [4 x [5 x i32]]], [3 x [4 x [5 x i32]]]* %7, i64 3
  %arrayidx5 = getelementptr inbounds [3 x [4 x [5 x i32]]], [3 x [4 x [5 x i32]]]* %arrayidx, i64 0, i64 1
  %8 = load i32, i32* %j, align 4
  %idxprom = sext i32 %8 to i64
  %arrayidx6 = getelementptr inbounds [4 x [5 x i32]], [4 x [5 x i32]]* %arrayidx5, i64 0, i64 %idxprom
  %9 = load i32, i32* %k, align 4
  %idxprom7 = sext i32 %9 to i64
  %arrayidx8 = getelementptr inbounds [5 x i32], [5 x i32]* %arrayidx6, i64 0, i64 %idxprom7
  %10 = load i32, i32* %arrayidx8, align 4
  %add9 = add nsw i32 %10, 1
  store i32 %add9, i32* %arrayidx8, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body4
  %11 = load i32, i32* %k, align 4
  %inc = add nsw i32 %11, 1
  store i32 %inc, i32* %k, align 4
  br label %for.cond2

for.end:                                          ; preds = %for.cond2
  br label %for.inc10

for.inc10:                                        ; preds = %for.end
  %12 = load i32, i32* %j, align 4
  %inc11 = add nsw i32 %12, 1
  store i32 %inc11, i32* %j, align 4
  br label %for.cond

for.end12:                                        ; preds = %for.cond
  br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end12
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %13 = load i32, i32* %.omp.iv, align 4
  %add13 = add nsw i32 %13, 1
  store i32 %add13, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

; Zero-trip test for reduction array finalization
; CHECK-DAG: %[[GLOBAL:[a-zA-Z._0-9]+]] = load [3 x [4 x [5 x i32]]]*, [3 x [4 x [5 x i32]]]** @_Z9yarrayptr
; CHECK-DAG: %[[GLOBAL_CAST:[a-zA-Z._0-9]+]] = bitcast [3 x [4 x [5 x i32]]]* %[[GLOBAL]] to i32*
; CHECK-DAG: %[[GLOBAL_CAST_PLUS_OFFSET:[a-zA-Z._0-9]+]] = getelementptr i32, i32* %[[GLOBAL_CAST]], i64 211
; CHECK-DAG: %[[GLOBAL_END:[a-zA-Z._0-9]+]] = getelementptr i32, i32* %[[GLOBAL_CAST_PLUS_OFFSET]], i64 6
; CHECK-DAG: %{{[a-zA-Z._0-9]+}} = icmp eq i32* %[[GLOBAL_CAST_PLUS_OFFSET]], %[[GLOBAL_END]]

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
