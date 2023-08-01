; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing' -S %s | FileCheck %s
;
; Original code:
;
;struct C {
;  int x;
;  C() = default;
;  C(const C &other) : x(other.x) {}
;};
;void foo(C c1, C c2) {
;#pragma omp target private(c1) firstprivate(c2)
;  (void)&c1;(void)&c2;
;}

; CHECK: "DIR.OMP.TARGET"()
; CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD.TYPED.WILOCAL"(ptr addrspace(4) %c1, %struct.C zeroinitializer, i32 1, ptr @_ZTS1C.omp.def_constr, ptr @_ZTS1C.omp.destr),
; CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:NONPOD.TYPED.WILOCAL"(ptr addrspace(4) %c2, %struct.C zeroinitializer, i32 1, ptr @_ZTS1C.omp.copy_constr, ptr @_ZTS1C.omp.destr) ]

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%struct.C = type { i32 }

define hidden spir_func void @_Z3foo1CS_(ptr addrspace(4) %c1, ptr addrspace(4) %c2) {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr addrspace(4) %c1, %struct.C zeroinitializer, i32 1, ptr @_ZTS1C.omp.def_constr, ptr @_ZTS1C.omp.destr),
    "QUAL.OMP.FIRSTPRIVATE:NONPOD.TYPED"(ptr addrspace(4) %c2, %struct.C zeroinitializer, i32 1, ptr @_ZTS1C.omp.copy_constr, ptr @_ZTS1C.omp.destr) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare spir_func ptr addrspace(4) @_ZTS1C.omp.def_constr(ptr addrspace(4) %0)
declare spir_func void @_ZTS1C.omp.destr(ptr addrspace(4) %0)
declare void @_ZTS1C.omp.copy_constr(ptr addrspace(4) %0, ptr addrspace(4) %1)
declare spir_func void @_ZN1CC2ERKS_(ptr addrspace(4) align 4 dereferenceable_or_null(4) %this, ptr addrspace(4) align 4 dereferenceable(4) %other)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2053, i32 11930794, !"_Z3foo1CS_", i32 7, i32 0, i32 0}
