; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Test src:
;
; void byref_reduction_simple(int (&y_ref)) {
;   #pragma omp parallel for reduction(+:y_ref)
;   for (int i = 0; i < 3; i++) {
;     y_ref += 1;
;   }
; }
;
; ModuleID = 'byref_reduction_simple.cpp'
source_filename = "byref_reduction_simple.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z22byref_reduction_simpleRi(i32* dereferenceable(4) %y_ref) #0 {
entry:
  %y_ref.addr = alloca i32*, align 8
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %i = alloca i32, align 4
  store i32* %y_ref, i32** %y_ref.addr, align 8
  store i32 0, i32* %.omp.lb, align 4
  store i32 2, i32* %.omp.ub, align 4
  store i32 1, i32* %.omp.stride, align 4
  store i32 0, i32* %.omp.is_last, align 4
  %0 = load i32*, i32** %y_ref.addr, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD:BYREF"(i32** %y_ref.addr), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
; CHECK-NOT: call token @llvm.directive.region.entry()
; CHECK-NOT: call token @llvm.directive.region.exit()

; Save local array's address to a var which will replace uses of original var within the region.
; CHECK: %[[LOCAL:y_ref.addr.red]] = alloca i32
; CHECK: store i32* %[[LOCAL]], i32** %[[LOCAL_REF:[a-zA-Z._0-9]+]]

; Reduction initialization.
; CHECK: store i32 0, i32* %[[LOCAL]]

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
; Check for the replacement of original %y_byrefarg_ptr.addr with LOCAL_MINUS_OFFSET_ADDR_REF.
; CHECK: %{{[a-zA-Z._0-9]+}} = load i32*, i32** %[[LOCAL_REF]]
  %6 = load i32*, i32** %y_ref.addr, align 8
  %7 = load i32, i32* %6, align 4
  %add1 = add nsw i32 %7, 1
  store i32 %add1, i32* %6, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, i32* %.omp.iv, align 4
  %add2 = add nsw i32 %8, 1
  store i32 %add2, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
; Reduction finalization code
; CHECK %[[ORIG_REF_LOAD:[a-zA-Z._0-9]+]] = load i32*, i32** %y_ref.addr
; CHECK-NEXT %[[ORIG_LOAD:[a-zA-Z._0-9]+]] = load i32, i32* %[[ORIG_REF_LOAD]]
; CHECK-NEXT %[[LOCAL_LOAD:[a-zA-Z._0-9]+]] = load i32, i32* %[[LOCAL]]
; CHECK-NEXT %[[ADD:[a-zA-Z._0-9]+]] = add i32 %[[ORIG_LOAD]], %[[LOCAL_LOAD]]
; CHECK-NEXT store i32 %[[ADD]], i32* %[[ORIG_REF_LOAD]]

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
