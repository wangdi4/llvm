; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s 2>&1 | FileCheck %s

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

define hidden spir_func void @foo(float addrspace(4)* %p) {
entry:
  %p.addr = alloca float addrspace(4)*, align 8
  %p.addr.ascast = addrspacecast float addrspace(4)** %p.addr to float addrspace(4)* addrspace(4)*
  store float addrspace(4)* %p, float addrspace(4)* addrspace(4)* %p.addr.ascast, align 8
  %0 = load float addrspace(4)*, float addrspace(4)* addrspace(4)* %p.addr.ascast, align 8
  %1 = load float addrspace(4)*, float addrspace(4)* addrspace(4)* %p.addr.ascast, align 8
  %arrayidx = getelementptr inbounds float, float addrspace(4)* %1, i64 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(float addrspace(4)* %0, float addrspace(4)* %arrayidx, i64 4),
    "QUAL.OMP.PRIVATE"(float addrspace(4)* addrspace(4)* %p.addr.ascast) ]

  store float addrspace(4)* %0, float addrspace(4)* addrspace(4)* %p.addr.ascast
  %3 = load float addrspace(4)*, float addrspace(4)* addrspace(4)* %p.addr.ascast, align 8
  %4 = load float, float addrspace(4)* %3, align 4
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2052, i32 55994960, !"foo", i32 2, i32 0, i32 0}
