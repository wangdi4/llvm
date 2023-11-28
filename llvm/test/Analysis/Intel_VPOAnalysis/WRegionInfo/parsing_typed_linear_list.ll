; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s

; This code tests TYPED for the LINEAR clause
; The test is passed if LINEAR:TYPED clauses are parsed correctly

; Test src:
;
; #include <omp.h>
; int x[10];
; int y = 1;
; int z = 2;
; int main() {
;   int m = 3;
;   int arr[] = {0, 1, 2, 3, 4};
;   int *p = arr;
; #pragma omp for linear(y, z, m, p : 3)
;   for (int i = 0; i < 10; i++) {
;     x[i] = y + p[1];
;   }
; }

; CHECK: LINEAR clause (size=4): (TYPED({{.*}}, TYPE: i32, NUM_ELEMENTS: i32 1), i32 3) (TYPED(ptr @z, TYPE: i32, NUM_ELEMENTS: i32 1), i32 3) (TYPED(ptr %m, TYPE: i32, NUM_ELEMENTS: i32 1), i32 3) (TYPED(PTR_TO_PTR(ptr %p), TYPE: ptr, POINTEE_TYPE: i32, NUM_ELEMENTS: i32 1), i32 3)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x = dso_local global [10 x i32] zeroinitializer, align 16
@y = dso_local global i32 1, align 4
@z = dso_local global i32 2, align 4
@__const.main.arr = private unnamed_addr constant [5 x i32] [i32 0, i32 1, i32 2, i32 3, i32 4], align 16

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %m = alloca i32, align 4
  %arr = alloca [5 x i32], align 16
  %p = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  store i32 3, ptr %m, align 4
  call void @llvm.memcpy.p0.p0.i64(ptr align 16 %arr, ptr align 16 @__const.main.arr, i64 20, i1 false)
  %arraydecay = getelementptr inbounds [5 x i32], ptr %arr, i64 0, i64 0
  store ptr %arraydecay, ptr %p, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 9, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.LINEAR:TYPED"(ptr @y, i32 0, i32 1, i32 3),
    "QUAL.OMP.LINEAR:TYPED"(ptr @z, i32 0, i32 1, i32 3),
    "QUAL.OMP.LINEAR:TYPED"(ptr %m, i32 0, i32 1, i32 3),
    "QUAL.OMP.LINEAR:PTR_TO_PTR.TYPED"(ptr %p, i32 0, i32 1, i32 3),
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
  %5 = load i32, ptr @y, align 4
  %6 = load ptr, ptr %p, align 8
  %arrayidx = getelementptr inbounds i32, ptr %6, i64 1
  %7 = load i32, ptr %arrayidx, align 4
  %add1 = add nsw i32 %5, %7
  %8 = load i32, ptr %i, align 4
  %idxprom = sext i32 %8 to i64
  %arrayidx2 = getelementptr inbounds [10 x i32], ptr @x, i64 0, i64 %idxprom
  store i32 %add1, ptr %arrayidx2, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, ptr %.omp.iv, align 4
  %add3 = add nsw i32 %9, 1
  store i32 %add3, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]
  %10 = load i32, ptr %retval, align 4
  ret i32 %10
}

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nofree nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
