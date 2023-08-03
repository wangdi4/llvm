; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; #pragma omp declare target
; int my_glob[100];
;
; void foo(char *);
; #pragma omp end declare target
;
; void bar() {
;   int i;
; #pragma omp target map(my_glob[:10])
;   foo((char *)my_glob);
; }

; Check that @my_glob reference inside the region is correctly replaced
; with the argument of the outline function:
; CHECK: @my_glob = common dso_local
; CHECK: define {{.*}}spir_kernel void @__omp_offloading_804_5203cff_bar_l9(ptr addrspace(1) %my_glob)
; CHECK: [[MY_GLOB_CAST:%.+]] = addrspacecast ptr addrspace(1) %my_glob to ptr addrspace(4)
; CHECK: call spir_func void @foo(ptr addrspace(4) [[MY_GLOB_CAST]])

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

@my_glob = common dso_local target_declare addrspace(1) global [100 x i32] zeroinitializer, align 4

define dso_local spir_func void @bar() {
entry:
  %i = alloca i32, align 4
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @my_glob to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @my_glob to ptr addrspace(4)), i64 40, i64 35, ptr null, ptr null) ] ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)

  call spir_func void @foo(ptr addrspace(4) addrspacecast (ptr addrspace(1) bitcast (ptr addrspace(1) @my_glob to ptr addrspace(1)) to ptr addrspace(4)))
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local spir_func void @foo(ptr addrspace(4))

!omp_offload.info = !{!0, !1}
!0 = !{i32 0, i32 2052, i32 85998847, !"bar", i32 9, i32 1, i32 0}
!1 = !{i32 1, !"my_glob", i32 0, i32 0, ptr addrspace(1) @my_glob}
