; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s

; Test src:
;
; void arrsecred_ptr_ref(long *(&y_byrefarg_ptr)) {
; #pragma omp parallel for reduction(+:y_byrefarg_ptr[2])
;   for(int i = 0; i < 10; i++) {
;     y_byrefarg_ptr[2] += 1;
;   }
; }

; The test IR was hand-modified to use a constant section offset.
; CFE currently generates IR instructions to compute it.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z17arrsecred_ptr_refRPl(ptr dereferenceable(8) %y_byrefarg_ptr) #0 {
entry:
  %y_byrefarg_ptr.addr = alloca ptr, align 8
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr %y_byrefarg_ptr, ptr %y_byrefarg_ptr.addr, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 9, ptr %.omp.ub, align 4
  store i32 1, ptr %.omp.stride, align 4
  store i32 0, ptr %.omp.is_last, align 4
  %0 = load ptr, ptr %y_byrefarg_ptr.addr, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.ADD:BYREF.ARRSECT.PTR_TO_PTR.TYPED"(ptr %y_byrefarg_ptr.addr, i64 0, i64 1, i64 2),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

; CHECK-NOT: call token @llvm.directive.region.entry()
; CHECK-NOT: call token @llvm.directive.region.exit()
; Check for array section reduction local copy preparation
; CHECK-DAG: %[[LOCAL_MINUS_OFFSET:[a-zA-Z._0-9]+]] = getelementptr i64, ptr %[[LOCAL:[a-zA-Z._0-9]+]], i64 -2
; CHECK-DAG: store ptr %[[LOCAL_MINUS_OFFSET]], ptr %[[LOCAL_MINUS_OFFSET_ADDR:[a-zA-Z._0-9]+]]
; CHECK-DAG: store ptr %[[LOCAL_MINUS_OFFSET_ADDR]], ptr %[[LOCAL_MINUS_OFFSET_ADDR_REF:[a-zA-Z._0-9]+]]

; Zero-trip test for reduction array initialization
; CHECK-DAG: %[[LOCAL_END:[0-9]+]] = getelementptr i64, ptr %[[LOCAL]], i64 1
; CHECK-DAG: %{{[a-zA-Z._0-9]+}} = icmp eq ptr %[[LOCAL]], %[[LOCAL_END]]

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

; Check for the replacement of original %y_byrefarg_ptr.addr with LOCAL_MINUS_OFFSET_ADDR_REF.
; CHECK-DAG: %{{[a-zA-Z._0-9]+}} = load ptr, ptr %[[LOCAL_MINUS_OFFSET_ADDR_REF]]

  %6 = load ptr, ptr %y_byrefarg_ptr.addr, align 8
  %7 = load ptr, ptr %6, align 8
  %arrayidx = getelementptr inbounds i64, ptr %7, i64 2
  %8 = load i64, ptr %arrayidx, align 8
  %add1 = add nsw i64 %8, 1
  store i64 %add1, ptr %arrayidx, align 8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, ptr %.omp.iv, align 4
  %add2 = add nsw i32 %9, 1
  store i32 %add2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end

; Start and end of original reduction array section for finalization.
; CHECK-DAG: %[[ORIG_LOAD_LOAD_PLUS_OFFSET:[a-zA-Z._0-9]+]] = getelementptr i64, ptr %[[ORIG_LOAD_LOAD:[a-zA-Z._0-9]+]], i64 2
; CHECK-DAG: %[[ORIG_LOAD_LOAD]] = load ptr, ptr %[[ORIG_LOAD:[a-zA-Z._0-9]+]]
; CHECK-DAG: %[[ORIG_LOAD]] = load ptr, ptr %y_byrefarg_ptr.addr
; CHECK-DAG: %[[ORIG_END:[a-zA-Z._0-9]+]] = getelementptr i64, ptr %[[ORIG_LOAD_LOAD_PLUS_OFFSET]], i64 1
; CHECK-DAG: %{{[a-zA-Z._0-9]+}} = icmp eq ptr %[[ORIG_LOAD_LOAD_PLUS_OFFSET]], %[[ORIG_END]]

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

!0 = !{i32 1, !"wchar_size", i32 4}
