; RUN: opt < %s -switch-to-offload=true -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-cfg-restructuring -vpo-paropt  -S | FileCheck %s

; The test case checks whether the omp teams distribute parlalel loop
; is partitioned as expected.
; on top of OCL.
;
; void bar() {
;   #pragma omp target
;   #pragma omp teams
;   #pragma omp distribute parallel for
;   for (int j=0; j<100; j++) foo();
; }

target triple = "spir64"
target device_triples = "spir64"

@"@tid.addr" = external global i32

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @_Z3barv() #0 {
entry:
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %j = alloca i32, align 4
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.stride), "QUAL.OMP.PRIVATE"(i32* %.omp.is_last), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"() ]
  store i32 0, i32* %.omp.lb, align 4
  store volatile i32 99, i32* %.omp.ub, align 4
  store i32 1, i32* %.omp.stride, align 4
  store i32 0, i32* %.omp.is_last, align 4
  br label %DIR.OMP.DISTRIBUTE.PARLOOP.4

DIR.OMP.DISTRIBUTE.PARLOOP.4:                     ; preds = %DIR.OMP.TARGET.2
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %j) ]
  %3 = load i32, i32* %.omp.lb, align 4
  store volatile i32 %3, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.DISTRIBUTE.PARLOOP.4
  %4 = load volatile i32, i32* %.omp.iv, align 4
  %5 = load volatile i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.loop.exit

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load volatile i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %j, align 4
  call spir_func void @_Z3foov()
  %7 = load volatile i32, i32* %.omp.iv, align 4
  %add1 = add nsw i32 %7, 1
  store volatile i32 %add1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  br label %DIR.OMP.END.DISTRIBUTE.PARLOOP.6

DIR.OMP.END.DISTRIBUTE.PARLOOP.6:                 ; preds = %omp.loop.exit
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  br label %DIR.OMP.END.TEAMS.7

DIR.OMP.END.TEAMS.7:                              ; preds = %DIR.OMP.END.DISTRIBUTE.PARLOOP.6
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local spir_func void @_Z3foov() #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 58, i32 -1942149079, !"_Z3barv", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 8.0.0"}
!4 = distinct !{i32 0}

; CHECK: %{{.*}} = call i64 @_Z14get_num_groupsj(i32 0)
; CHECK: %{{.*}} = call i64 @_Z12get_group_idj(i32 0)
