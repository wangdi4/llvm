; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
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
; CHECK: call spir_func void @__kmpc_atomic_fixed4_add(ptr addrspace(4) %{{[0-9]+}}, i32 2)


target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr constant [7 x i8] c"aaa=%d\00", align 1

define dso_local i32 @main() {
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
  store i32 0, ptr %retval, align 4
  store i32 10, ptr %aaa, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr %aaa, ptr %aaa, i64 4, i64 35, ptr null, ptr null),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.stride, i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.is_last, i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i64 1) ]

  store i32 0, ptr %.omp.lb, align 4
  store i32 31, ptr %.omp.ub, align 4
  store i32 1, ptr %.omp.stride, align 4
  store i32 0, ptr %.omp.is_last, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.NUM_THREADS"(i32 32),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i64 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.SHARED:TYPED"(ptr %aaa, i32 0, i64 1) ]

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
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(),
    "QUAL.OMP.UPDATE"() ]

  fence acquire
  %7 = load i32, ptr %aaa, align 4
  %add1 = add nsw i32 %7, 2
  store i32 %add1, ptr %aaa, align 4
  fence release

  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.ATOMIC"() ]
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr %.omp.iv, align 4
  %add2 = add nsw i32 %8, 1
  store i32 %add2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  %9 = load i32, ptr %aaa, align 4
  %call = call i32 (ptr, ...) @printf(ptr @.str, i32 %9)
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(ptr, ...)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 59, i32 -696885959, !"main", i32 5, i32 0, i32 0}
