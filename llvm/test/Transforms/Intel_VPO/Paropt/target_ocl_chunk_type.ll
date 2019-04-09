; RUN: opt -switch-to-offload -vpo-paropt -S < %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -switch-to-offload -S < %s  | FileCheck %s

; void star1() {
;     #pragma omp target parallel for schedule(static,7)
;     for (long i=1; i<100; ++i) { }
; }
; Compiling it with -fiopenmp -fopenmp-targets=spir64 will require building
; MUL expressions involving the chunk size (i32 7) and other operands cast
; to IV's type (i64). Verify that this situation is properly handled and
; that no type-mismatch assertions occur.

; CHECK: [[NUMTHREADS:%[0-9]+]] = call i64 @_Z14get_local_sizej(i32 0)
; CHECK: %{{.*}} = mul i64 [[NUMTHREADS]], 7
;
; CHECK: [[LOCALID:%[0-9]+]] = call i64 @_Z12get_local_idj(i32 0)
; CHECK: %{{.*}} = mul i64 [[LOCALID]], 7

; ModuleID = 'tt.cpp'
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

@"@tid.addr" = external global i32

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @_Z5star1v() #0 {
entry:
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %.omp.iv = alloca i64, align 8
  %tmp = alloca i64, align 8
  %i = alloca i64, align 8
  store i64 0, i64* %.omp.lb, align 8
  store volatile i64 98, i64* %.omp.ub, align 8
  br label %DIR.OMP.TARGET.1.split

DIR.OMP.TARGET.1.split:                           ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(i64* %i), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.lb), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.ub), "QUAL.OMP.PRIVATE"(i64* %tmp) ]
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1.split
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.STATIC"(i32 7), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i64* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i64* %.omp.ub), "QUAL.OMP.PRIVATE"(i64* %i) ]
  br label %DIR.OMP.PARALLEL.LOOP.1

DIR.OMP.PARALLEL.LOOP.1:                          ; preds = %DIR.OMP.TARGET.2
  %2 = load i64, i64* %.omp.lb, align 8
  store volatile i64 %2, i64* %.omp.iv, align 8
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.PARALLEL.LOOP.1
  %3 = load volatile i64, i64* %.omp.iv, align 8
  %4 = load volatile i64, i64* %.omp.ub, align 8
  %cmp = icmp sle i64 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.loop.exit

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load volatile i64, i64* %.omp.iv, align 8
  %mul = mul nsw i64 %5, 1
  %add = add nsw i64 1, %mul
  store i64 %add, i64* %i, align 8
  %6 = load volatile i64, i64* %.omp.iv, align 8
  %add1 = add nsw i64 %6, 1
  store volatile i64 %add1, i64* %.omp.iv, align 8
  br label %omp.inner.for.cond

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.LOOP.4

DIR.OMP.END.PARALLEL.LOOP.4:                      ; preds = %omp.loop.exit
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.2

DIR.OMP.END.TARGET.2:                             ; preds = %DIR.OMP.END.PARALLEL.LOOP.4
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}

!0 = !{i32 0, i32 58, i32 -694023166, !"_Z5star1v", i32 2, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
