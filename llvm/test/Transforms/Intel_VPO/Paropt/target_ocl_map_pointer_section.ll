; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s 2>&1 | FileCheck %s

; Check that all loads/stores to/from the global private version of the address
; of %p were transformed to use addrspace(1) directly.
; CHECK: [[PRIV_GLOB:@[a-zA-Z._0-9]+]] = internal addrspace(1) global float
; CHECK: store float addrspace(1)*{{.*}}, float addrspace(1)* addrspace(1)* [[PRIV_GLOB]]
; CHECK: load float addrspace(1)*, float addrspace(1)* addrspace(1)* [[PRIV_GLOB]]
; CHECK-NOT: store float{{.*}}addrspace(4){{.*}}[[PRIV_GLOB]]
; CHECK-NOT: load float addrspace(4){{.*}}[[PRIV_GLOB]]

; Original code:
; void foo(float *p) {
; #pragma omp target map(p[:1])
;   *p;
; }

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define hidden spir_func void @foo(float addrspace(4)* %p) #0 {
entry:
  %p.addr = alloca float addrspace(4)*, align 8
  %p.addr.ascast = addrspacecast float addrspace(4)** %p.addr to float addrspace(4)* addrspace(4)*
  store float addrspace(4)* %p, float addrspace(4)* addrspace(4)* %p.addr.ascast, align 8
  %0 = load float addrspace(4)*, float addrspace(4)* addrspace(4)* %p.addr.ascast, align 8
  %1 = load float addrspace(4)*, float addrspace(4)* addrspace(4)* %p.addr.ascast, align 8
  %arrayidx = getelementptr inbounds float, float addrspace(4)* %1, i64 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(float addrspace(4)* %0, float addrspace(4)* %arrayidx, i64 4), "QUAL.OMP.PRIVATE"(float addrspace(4)* addrspace(4)* %p.addr.ascast) ]
  store float addrspace(4)* %0, float addrspace(4)* addrspace(4)* %p.addr.ascast
  %3 = load float addrspace(4)*, float addrspace(4)* addrspace(4)* %p.addr.ascast, align 8
  %4 = load float, float addrspace(4)* %3, align 4
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2052, i32 55994960, !"foo", i32 2, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 9.0.0"}
