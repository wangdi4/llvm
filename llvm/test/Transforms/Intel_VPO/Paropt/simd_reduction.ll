; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL
; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=FASTRED
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=FASTRED

; Original code:
; void foo()
; {
;   int l = 0;
; #pragma omp simd reduction(+:l)
;   for (i = 0; i < 1000; ++i)
;     ++l;
; }

; CRITICAL: [[ZTT:%.+]] = icmp sle i32 0, %{{.+}}
; CRITICAL: br i1 [[ZTT]], label %[[PHB:[^,]+]], label %[[REXIT:[^,]+]]
; CRITICAL: [[PHB]]:
; CRITICAL: [[TOK:%.+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),{{.*}}"QUAL.OMP.REDUCTION.ADD"(i32* %[[RPRIV:[^,]+]]){{.*}} ]
; CRITICAL: br label %[[LOOPBODY:[^,]+]]
; CRITICAL: [[LOOPBODY]]:
; CRITICAL: load {{.*}}i32* %[[RPRIV]]
; CRITICAL: store {{.*}}i32* %[[RPRIV]]
; CRITICAL: br i1 %{{[^,]+}}, label %[[LOOPBODY]], label %[[LEXIT:[^,]+]]
; CRITICAL: [[LEXIT]]:
; CRITICAL: call void @llvm.directive.region.exit(token [[TOK]]) [ "DIR.OMP.END.SIMD"() ]
; CRITICAL: br label %[[LEXIT_SPLIT:[^,]+]]
; CRITICAL: [[LEXIT_SPLIT]]:
; CRITICAL: br label %[[LEXIT_SPLIT_SPLIT:[^,]+]]
; CRITICAL: [[LEXIT_SPLIT_SPLIT]]:
; CRITICAL: %[[V:.+]] = load i32, i32* %[[RPRIV]]
; CRITICAL: %[[OV:.+]] = load i32, i32* %l
; CRITICAL: %[[ADD:.+]] = add i32 %[[OV]], %[[V]]
; CRITICAL: store i32 %[[ADD]], i32* %l
; CRITICAL: br label %[[REXIT]]
; CRITICAL: [[REXIT]]:

; FASTRED: [[ZTT:%.+]] = icmp sle i32 0, %{{.+}}
; FASTRED: br i1 [[ZTT]], label %[[PHB:[^,]+]], label %[[REXIT:[^,]+]]
; FASTRED: [[PHB]]:
; FASTRED: [[TOK:%.+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),{{.*}}"QUAL.OMP.REDUCTION.ADD"(i32* %[[RPRIV:[^,]+]]){{.*}} ]
; FASTRED: br label %[[LOOPBODY:[^,]+]]
; FASTRED: [[LOOPBODY]]:
; FASTRED: load {{.*}}i32* %[[RPRIV]]
; FASTRED: store {{.*}}i32* %[[RPRIV]]
; FASTRED: br i1 %{{[^,]+}}, label %[[LOOPBODY]], label %[[LEXIT:[^,]+]]
; FASTRED: [[LEXIT]]:
; FASTRED: call void @llvm.directive.region.exit(token [[TOK]]) [ "DIR.OMP.END.SIMD"() ]
; FASTRED: %[[R:.+]] = load i32, i32* %[[RPRIV]]
; FASTRED: store i32 %[[R]], i32* %[[FRPRIV:.+]]
; FASTRED: br label %[[LEXIT_SPLIT:[^,]+]]
; FASTRED: [[LEXIT_SPLIT]]:
; FASTRED: br label %[[LEXIT_SPLIT_SPLIT:[^,]+]]
; FASTRED: [[LEXIT_SPLIT_SPLIT]]:
; FASTRED: %[[V:.+]] = load i32, i32* %[[FRPRIV]]
; FASTRED: %[[OV:.+]] = load i32, i32* %l
; FASTRED: %[[ADD:.+]] = add i32 %[[OV]], %[[V]]
; FASTRED: store i32 %[[ADD]], i32* %l
; FASTRED: br label %[[REXIT]]
; FASTRED: [[REXIT]]:

; ModuleID = 'simd.cpp'
source_filename = "simd.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @_Z3foov() #0 {
entry:
  %l = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, i32* %l, align 4
  store i32 999, i32* %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD"(i32* %l), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub) ]
  store i32 0, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %1 = load i32, i32* %.omp.iv, align 4
  %2 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %1, %2
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %3 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %3, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %4 = load i32, i32* %l, align 4
  %inc = add nsw i32 %4, 1
  store i32 %inc, i32* %l, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %5 = load i32, i32* %.omp.iv, align 4
  %add1 = add nsw i32 %5, 1
  store i32 %add1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
