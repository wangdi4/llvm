; RUN: opt < %s -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -simplifycfg  -sroa -vpo-cfg-restructuring -vpo-paropt -switch-to-offload -S | FileCheck %s
; RUN: opt < %s -passes='function(loop(rotate),vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,simplify-cfg,loop(simplify-cfg),sroa,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S | FileCheck %s

; Original code:
; void foo() {
;   int i;
; #pragma omp target teams distribute parallel for schedule(static, 1)
;   for (i = 0; i < 1000; ++i);
; }

; Check that team.ub is used to compute the partitioned loop's
; upper bound.  The check below is not very robust, because
; it relies on temp/block names:
; CHECK: define{{.*}}@__omp_offloading_804_5200f43_foo_l3
; CHECK: %[[TEAM_UB:.*]] = load i32, i32* %team.ub
; CHECK: dispatch.header:
; CHECK: %[[UB_TMP:.*]] = load i32, i32*
; CHECK: %[[UB_MIN:.*]] = icmp sle i32 %[[UB_TMP]], %[[TEAM_UB]]
; CHECK: br i1 %[[UB_MIN]], label %[[DISP_BODY:.*]], label %[[DISP_MIN:.*]]
; CHECK: [[DISP_MIN]]:
; CHECK: store i32 %[[TEAM_UB]], i32*

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @foo() #0 {
entry:
  %i = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  store i32 0, i32* %.omp.lb, align 4
  store i32 999, i32* %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.ub), "QUAL.OMP.FIRSTPRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %tmp) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.SHARED"(i32* %.omp.lb), "QUAL.OMP.SHARED"(i32* %.omp.ub), "QUAL.OMP.SHARED"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %tmp) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.SCHEDULE.STATIC"(i32 1), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
  %3 = load i32, i32* %.omp.lb, align 4
  store i32 %3, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, i32* %.omp.iv, align 4
  %5 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, i32* %.omp.iv, align 4
  %add1 = add nsw i32 %7, 1
  store i32 %add1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2052, i32 85987139, !"foo", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 9.0.0"}
