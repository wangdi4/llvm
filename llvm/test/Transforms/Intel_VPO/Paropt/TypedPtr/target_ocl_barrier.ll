; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; void foo(int *x) {
; #pragma omp target parallel map(x[:1])
;  {
;     *x = 1;
; #pragma omp barrier
;     ;
;   }
; }

; Check that __kmpc_barrier is called:
; CHECK: declare spir_func void @__kmpc_barrier() #[[ATTR:[0-9]+]]
; CHECK: call spir_func void @__kmpc_barrier
; CHECK: attributes #[[ATTR]] = {{{.*}}convergent{{.*}}}

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

define hidden spir_func void @foo(i32 addrspace(4)* %x) {
entry:
  %x.addr = alloca i32 addrspace(4)*, align 8
  %x.addr.ascast = addrspacecast i32 addrspace(4)** %x.addr to i32 addrspace(4)* addrspace(4)*
  %x.map.ptr.tmp = alloca i32 addrspace(4)*, align 8
  %x.map.ptr.tmp.ascast = addrspacecast i32 addrspace(4)** %x.map.ptr.tmp to i32 addrspace(4)* addrspace(4)*
  store i32 addrspace(4)* %x, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %0 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %1 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %arrayidx = getelementptr inbounds i32, i32 addrspace(4)* %1, i64 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(i32 addrspace(4)* %0, i32 addrspace(4)* %arrayidx, i64 4),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast) ]

  store i32 addrspace(4)* %0, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast) ]

  %4 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8
  store i32 1, i32 addrspace(4)* %4, align 4
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.BARRIER"() ]
  fence acq_rel
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.BARRIER"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2052, i32 51517201, !"foo", i32 2, i32 0, i32 0}
