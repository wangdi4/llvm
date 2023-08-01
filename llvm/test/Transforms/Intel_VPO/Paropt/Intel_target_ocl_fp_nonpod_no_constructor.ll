; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -S %s | FileCheck %s
;
; Original code:
;
;struct C {
;  int x;
;  C() = default;
;  C(const C &other) : x(other.x) {}
;};
;void foo(C c1) {
;#pragma omp target firstprivate(c1)
;  (void)&c1;
;}

;CHECK-NOT: call{{.*}}@_ZTS1C.omp.copy_constr
;CHECK-NOT: call{{.*}}@_ZTS1C.omp.destr

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%struct.C = type { i32 }

$_ZN1CC2ERKS_ = comdat any

define hidden spir_func void @_Z3foo1C(ptr addrspace(4) %c1) {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:NONPOD.TYPED"(ptr addrspace(4) %c1, %struct.C zeroinitializer, i32 1, ptr @_ZTS1C.omp.copy_constr, ptr @_ZTS1C.omp.destr),
    "QUAL.OMP.MAP.TO"(ptr addrspace(4) %c1, ptr addrspace(4) %c1, i64 4, i64 161, ptr null, ptr null) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @_ZTS1C.omp.copy_constr(ptr addrspace(4) %0, ptr addrspace(4) %1)
declare spir_func void @_ZN1CC2ERKS_(ptr addrspace(4) align 4 dereferenceable_or_null(4) %this, ptr addrspace(4) align 4 dereferenceable(4) %other)
declare spir_func void @_ZTS1C.omp.destr(ptr addrspace(4) %0)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2053, i32 13639481, !"_Z3foo1C", i32 7, i32 0, i32 0}
