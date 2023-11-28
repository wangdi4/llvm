; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s

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

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z21byref_reduction_arrayRA3_A4_A5_i(ptr noundef nonnull align 4 dereferenceable(240) %y_Arr_ref) #0 {
entry:
  %y_Arr_ref.addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %k = alloca i32, align 4
  store ptr %y_Arr_ref, ptr %y_Arr_ref.addr, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 2, ptr %.omp.ub, align 4
  %0 = load ptr, ptr %y_Arr_ref.addr, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.ADD:BYREF.TYPED"(ptr %y_Arr_ref.addr, [3 x [4 x [5 x i32]]] zeroinitializer, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %k, i32 0, i32 1) ]

; CHECK-NOT: call token @llvm.directive.region.entry()
; CHECK-NOT: call token @llvm.directive.region.exit()

; Zero-trip test for reduction array initialization
; CHECK-DAG: %[[LOCAL_END:[a-zA-Z._0-9]+]] = getelementptr i32, ptr %[[LOCAL_BEGIN:[a-zA-Z._0-9]+]], i32 60
; CHECK-DAG: %[[LOCAL_BEGIN]] = getelementptr inbounds [3 x [4 x [5 x i32]]], ptr %[[LOCAL:[a-zA-Z._0-9]+]], i32 0, i32 0, i32 0, i32 0
; CHECK-DAG: %{{[a-zA-Z._0-9]+}} = icmp eq ptr %[[LOCAL_BEGIN]], %[[LOCAL_END]]

; Save local array's address to a var which will replace uses of original var within the region.
; CHECK-DAG: store ptr %[[LOCAL]], ptr %[[LOCAL_REF:[a-zA-Z._0-9]+]]

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
  store i32 0, ptr %j, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc8, %omp.inner.for.body
  %6 = load i32, ptr %j, align 4
  %cmp1 = icmp slt i32 %6, 4
  br i1 %cmp1, label %for.body, label %for.end10

for.body:                                         ; preds = %for.cond
  store i32 0, ptr %k, align 4
  br label %for.cond2

for.cond2:                                        ; preds = %for.inc, %for.body
  %7 = load i32, ptr %k, align 4
  %cmp3 = icmp slt i32 %7, 5
  br i1 %cmp3, label %for.body4, label %for.end

for.body4:                                        ; preds = %for.cond2
; Check for the replacement of original %y_Arr_ref.addr with LOCAL_REF.
; CHECK-DAG: %{{[a-zA-Z._0-9]+}} = load ptr, ptr %[[LOCAL_REF]]
  %8 = load ptr, ptr %y_Arr_ref.addr, align 8
  %arrayidx = getelementptr inbounds [3 x [4 x [5 x i32]]], ptr %8, i64 0, i64 1
  %arrayidx5 = getelementptr inbounds [4 x [5 x i32]], ptr %arrayidx, i64 0, i64 2
  %9 = load i32, ptr %k, align 4
  %idxprom = sext i32 %9 to i64
  %arrayidx6 = getelementptr inbounds [5 x i32], ptr %arrayidx5, i64 0, i64 %idxprom
  %10 = load i32, ptr %arrayidx6, align 4
  %add7 = add nsw i32 %10, 1
  store i32 %add7, ptr %arrayidx6, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body4
  %11 = load i32, ptr %k, align 4
  %inc = add nsw i32 %11, 1
  store i32 %inc, ptr %k, align 4
  br label %for.cond2, !llvm.loop !5

for.end:                                          ; preds = %for.cond2
  br label %for.inc8

for.inc8:                                         ; preds = %for.end
  %12 = load i32, ptr %j, align 4
  %inc9 = add nsw i32 %12, 1
  store i32 %inc9, ptr %j, align 4
  br label %for.cond, !llvm.loop !7

for.end10:                                        ; preds = %for.cond
  br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end10
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %13 = load i32, ptr %.omp.iv, align 4
  %add11 = add nsw i32 %13, 1
  store i32 %add11, ptr %.omp.iv, align 4
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

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}
!7 = distinct !{!7, !6}
