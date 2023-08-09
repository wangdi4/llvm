; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes="function(vpo-cfg-restructuring),vpo-paropt" -S %s | FileCheck %s

; Test src:

; short x;
; a() {
; short y;
; #pragma omp target private(x, y)
;   x = y;
; }

; Check that we are able to privatize x and y.

; CHECK: [[XP:@.*priv.__global]] = internal addrspace(1) global i16 0
; CHECK: [[YP:@.*y.ascast.priv.__global]] = internal addrspace(1) global i16 0

; CHECK: [[YP_VAL:%.+]] = load i16, ptr addrspace(1) [[YP]], align 2
; CHECK: store i16 [[YP_VAL]], ptr addrspace(1) [[XP]], align 2

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@x = external addrspace(1) global i16, align 2

; Function Attrs: convergent noinline nounwind optnone
define protected spir_func i32 @a() #0 {
entry:
  %retval = alloca i32, align 4
  %y = alloca i16, align 2
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %y.ascast = addrspacecast ptr %y to ptr addrspace(4)

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @x to ptr addrspace(4)), i16 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %y.ascast, i16 0, i32 1) ]

  %1 = load i16, ptr addrspace(4) %y.ascast, align 2
  store i16 %1, ptr addrspace(4) addrspacecast (ptr addrspace(1) @x to ptr addrspace(4)), align 2

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  %2 = load i32, ptr addrspace(4) %retval.ascast, align 4
  ret i32 %2
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66313, i32 47881402, !"_Z1a", i32 4, i32 0, i32 0}
