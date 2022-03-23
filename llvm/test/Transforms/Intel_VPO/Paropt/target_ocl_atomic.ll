; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; GPU-offload test for Atomic. The test is created by compiling
; the C test below with:  icx -O0 -fiopenmp -fopenmp-targets=spir64 -Qoption,cpp,-fintel-openmp-region-atomic
; int main() {
;   int i, aaa = 10;
;   #pragma omp target map(tofrom:aaa) private(i)
;   #pragma omp parallel for num_threads(32) shared(aaa) private (i)
;   for (i=0; i<32; i++) {
;     #pragma omp atomic
;     aaa = aaa+2;
;   }
;   printf("aaa=%d",aaa);
;   return 0;
; }

; The GPU-offloading compilation will:
;
; 1. Remove the fence acquire/release that Clang added to the Atomic region
; CHECK-NOT: fence acquire
; CHECK-NOT: fence release
;
; 2. Emit the kmpc atomic call without the ident_t and the global_id parameters
; 3. Add addrspace(4) to the pointer parameter
; CHECK: call spir_func void @__kmpc_atomic_fixed4_add(i32 addrspace(4)* %{{[0-9]+}}, i32 2)


; ModuleID = '<stdin>'
source_filename = "atomic.c"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr constant [7 x i8] c"aaa=%d\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %aaa = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 10, i32* %aaa, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(i32* %aaa), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.stride), "QUAL.OMP.PRIVATE"(i32* %.omp.is_last), "QUAL.OMP.PRIVATE"(i32* %tmp) ]
  store i32 0, i32* %.omp.lb, align 4
  store i32 31, i32* %.omp.ub, align 4
  store i32 1, i32* %.omp.stride, align 4
  store i32 0, i32* %.omp.is_last, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.NUM_THREADS"(i32 32), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.SHARED"(i32* %aaa) ]
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
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
  fence acquire
  %7 = load i32, i32* %aaa, align 4
  %add1 = add nsw i32 %7, 2
  store i32 %add1, i32* %aaa, align 4
  fence release
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.ATOMIC"() ]
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
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  %9 = load i32, i32* %aaa, align 4
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %9)
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 59, i32 -696885959, !"main", i32 5, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 8.0.0"}
