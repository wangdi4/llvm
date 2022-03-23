; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s

;
; Test src:
;
; void arrsecred_ptr_ref(long *(&y_byrefarg_ptr)) {
; #pragma omp parallel for reduction(+:y_byrefarg_ptr[2])
;   for(int i = 0; i < 10; i++) {
;     y_byrefarg_ptr[2] += 1;
;   }
; }
;
; ModuleID = 'arrsecred_ptrref.cpp'
source_filename = "arrsecred_ptrref.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z17arrsecred_ptr_refRPl(i64** dereferenceable(8) %y_byrefarg_ptr) #0 {
entry:
  %y_byrefarg_ptr.addr = alloca i64**, align 8
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %i = alloca i32, align 4
  store i64** %y_byrefarg_ptr, i64*** %y_byrefarg_ptr.addr, align 8
  store i32 0, i32* %.omp.lb, align 4
  store i32 9, i32* %.omp.ub, align 4
  store i32 1, i32* %.omp.stride, align 4
  store i32 0, i32* %.omp.is_last, align 4
  %0 = load i64**, i64*** %y_byrefarg_ptr.addr, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD:BYREF.ARRSECT"(i64*** %y_byrefarg_ptr.addr, i64 1, i64 2, i64 1, i64 1), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
  %2 = load i32, i32* %.omp.lb, align 4
; CHECK-NOT: call token @llvm.directive.region.entry()
; CHECK-NOT: call token @llvm.directive.region.exit()
; Check for array section reduction local copy preparation
; CHECK-DAG: %[[LOCAL_MINUS_OFFSET:[a-zA-Z._0-9]+]] = getelementptr i64, i64* %[[LOCAL:[a-zA-Z._0-9]+]], i64 -2
; CHECK-DAG: store i64* %[[LOCAL_MINUS_OFFSET]], i64** %[[LOCAL_MINUS_OFFSET_ADDR:[a-zA-Z._0-9]+]]
; CHECK-DAG: store i64** %[[LOCAL_MINUS_OFFSET_ADDR]], i64*** %[[LOCAL_MINUS_OFFSET_ADDR_REF:[a-zA-Z._0-9]+]]

; Zero-trip test for reduction array initialization
; CHECK-DAG: %[[LOCAL_END:[0-9]+]] = getelementptr i64, i64* %[[LOCAL]], i64 1
; CHECK-DAG: %{{[a-zA-Z._0-9]+}} = icmp eq i64* %[[LOCAL]], %[[LOCAL_END]]
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
; Check for the replacement of original %y_byrefarg_ptr.addr with LOCAL_MINUS_OFFSET_ADDR_REF.
; CHECK-DAG: %{{[a-zA-Z._0-9]+}} = load i64**, i64*** %[[LOCAL_MINUS_OFFSET_ADDR_REF]]
  %6 = load i64**, i64*** %y_byrefarg_ptr.addr, align 8
  %7 = load i64*, i64** %6, align 8
  %arrayidx = getelementptr inbounds i64, i64* %7, i64 2
  %8 = load i64, i64* %arrayidx, align 8
  %add1 = add nsw i64 %8, 1
  store i64 %add1, i64* %arrayidx, align 8
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
; Start and end of original reduction array section for finalization.
; CHECK-DAG: %[[ORIG_LOAD_LOAD_PLUS_OFFSET:[a-zA-Z._0-9]+]] = getelementptr i64, i64* %[[ORIG_LOAD_LOAD:[a-zA-Z._0-9]+]], i64 2
; CHECK-DAG: %[[ORIG_LOAD_LOAD]] = load i64*, i64** %[[ORIG_LOAD:[a-zA-Z._0-9]+]]
; CHECK-DAG: %[[ORIG_LOAD]] = load i64**, i64*** %y_byrefarg_ptr.addr
; CHECK-DAG: %[[ORIG_END:[a-zA-Z._0-9]+]] = getelementptr i64, i64* %[[ORIG_LOAD_LOAD_PLUS_OFFSET]], i64 1
; CHECK-DAG: %{{[a-zA-Z._0-9]+}} = icmp eq i64* %[[ORIG_LOAD_LOAD_PLUS_OFFSET]], %[[ORIG_END]]

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
