; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-cfg-restructuring -vpo-paropt -S < %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-cfg-restructuring),vpo-paropt' -S < %s  | FileCheck %s
;
; Test src:
;
; #include <cstdio>
;
; static short *yptr;
;
; void arrsecred_ptr() {
;   #pragma omp parallel for reduction(+ : yptr [6:3])
;   for (int i = 6; i < 9; i++) {
;     yptr[i] += 1;
;   }
; }
;
; ModuleID = 'arrsecred_ptr.cpp'
source_filename = "arrsecred_ptr.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@_Z4yptr = internal global i16* null, align 8

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z13arrsecred_ptrv() #0 {
entry:
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, i32* %.omp.lb, align 4
  store i32 2, i32* %.omp.ub, align 4
  store i32 1, i32* %.omp.stride, align 4
  store i32 0, i32* %.omp.is_last, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD:ARRSECT"(i16** @_Z4yptr, i64 1, i64 6, i64 3, i64 1), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
; CHECK-NOT: call token @llvm.directive.region.entry()
; CHECK-NOT: call token @llvm.directive.region.exit()
; Check for array section reduction local copy preparation
; CHECK-DAG: %[[LOCAL_MINUS_OFFSET:[a-zA-Z._0-9]+]] = getelementptr i16, i16* %[[LOCAL:[a-zA-Z._0-9]+]], i64 -6
; CHECK-DAG: store i16* %[[LOCAL_MINUS_OFFSET]], i16** %[[LOCAL_MINUS_OFFSET_ADDR:[a-zA-Z._0-9]+]]
; Zero-trip test for reduction array initialization
; CHECK-DAG: %[[LOCAL_END:[0-9]+]] = getelementptr i16, i16* %[[LOCAL]], i64 3
; CHECK-DAG: %{{[a-zA-Z._0-9]+}} = icmp eq i16* %[[LOCAL]], %[[LOCAL_END]]
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
  %add = add nsw i32 6, %mul
  store i32 %add, i32* %i, align 4
  %5 = load i16*, i16** @_Z4yptr, align 8
; Check for the replacement of original @_Z4yptr with "%y_local - offset".
; CHECK-DAG: %{{[0-9]+}} = load i16*, i16** %[[LOCAL_MINUS_OFFSET_ADDR]]
  %6 = load i32, i32* %i, align 4
  %idxprom = sext i32 %6 to i64
  %arrayidx = getelementptr inbounds i16, i16* %5, i64 %idxprom
  %7 = load i16, i16* %arrayidx, align 2
  %conv = sext i16 %7 to i32
  %add1 = add nsw i32 %conv, 1
  %conv2 = trunc i32 %add1 to i16
  store i16 %conv2, i16* %arrayidx, align 2
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, i32* %.omp.iv, align 4
  %add3 = add nsw i32 %8, 1
  store i32 %add3, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

; Zero-trip test for reduction array finalization
; CHECK-DAG: %[[GLOBAL:[a-zA-Z._0-9]+]] = load i16*, i16** @_Z4yptr
; CHECK-DAG: %[[GLOBAL_PLUS_OFFSET:[a-zA-Z._0-9]+]] = getelementptr i16, i16* %[[GLOBAL]], i64 6
; CHECK-DAG: %[[GLOBAL_END:[a-zA-Z._0-9]+]] = getelementptr i16, i16* %[[GLOBAL_PLUS_OFFSET]], i64 3
; CHECK-DAG: %{{[a-zA-Z._0-9]+}} = icmp eq i16* %[[GLOBAL_PLUS_OFFSET]], %[[GLOBAL_END]]

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
