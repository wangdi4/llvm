; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Original code:
; void foo() {
;   float x;
;   double y;
;   int i;
;   long long int j;
;   char c;
; #pragma omp target firstprivate(x, y, i, j, c)
;   {
;     (void)x;(void)y;(void)i;(void)j;(void)c;
;   }
; }

; Check that all firstprivate values are passed by value:
; CHECK: define{{.*}}void @__omp_offloading{{.*}}Z3foo_l7(i64{{[^*,]*}}, i64{{[^*,]*}}, i64{{[^*,]*}}, i64{{[^*,]*}}, i64{{[^*,]*}})

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define hidden spir_func void @foo() {
entry:
  %x = alloca float, align 4
  %x.ascast = addrspacecast ptr %x to ptr addrspace(4)
  %y = alloca double, align 8
  %y.ascast = addrspacecast ptr %y to ptr addrspace(4)
  %i = alloca i32, align 4
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %j = alloca i64, align 8
  %j.ascast = addrspacecast ptr %j to ptr addrspace(4)
  %c = alloca i8, align 1
  %c.ascast = addrspacecast ptr %c to ptr addrspace(4)
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %x.ascast, float 0.000000e+00, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %y.ascast, double 0.000000e+00, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i64 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %c.ascast, i8 0, i32 1) ]

  %1 = load float, ptr addrspace(4) %x.ascast, align 4
  %2 = load double, ptr addrspace(4) %y.ascast, align 8
  %3 = load i32, ptr addrspace(4) %i.ascast, align 4
  %4 = load i64, ptr addrspace(4) %j.ascast, align 8
  %5 = load i8, ptr addrspace(4) %c.ascast, align 1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2054, i32 1856547, !"_Z3foo", i32 7, i32 0, i32 0}
