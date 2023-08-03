; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -switch-to-offload -passes='vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing' -S %s | FileCheck %s
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
; CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD.WILOCAL"(%struct.C addrspace(4)* %c1, %struct.C addrspace(4)* (%struct.C addrspace(4)*)* @_ZTS1C.omp.def_constr, void (%struct.C addrspace(4)*)* @_ZTS1C.omp.destr)
; CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:NONPOD.WILOCAL"(%struct.C addrspace(4)* %c2, void (%struct.C addrspace(4)*, %struct.C addrspace(4)*)* @_ZTS1C.omp.copy_constr, void (%struct.C addrspace(4)*)* @_ZTS1C.omp.destr)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%struct.C = type { i32 }

define hidden spir_func void @_Z3foo1CS_(%struct.C addrspace(4)* %c1, %struct.C addrspace(4)* %c2) {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:NONPOD"(%struct.C addrspace(4)* %c1, %struct.C addrspace(4)* (%struct.C addrspace(4)*)* @_ZTS1C.omp.def_constr, void (%struct.C addrspace(4)*)* @_ZTS1C.omp.destr),
    "QUAL.OMP.FIRSTPRIVATE:NONPOD"(%struct.C addrspace(4)* %c2, void (%struct.C addrspace(4)*, %struct.C addrspace(4)*)* @_ZTS1C.omp.copy_constr, void (%struct.C addrspace(4)*)* @_ZTS1C.omp.destr),
    "QUAL.OMP.MAP.TO"(%struct.C addrspace(4)* %c2, %struct.C addrspace(4)* %c2, i64 4, i64 161, i8* null, i8* null) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare spir_func %struct.C addrspace(4)* @_ZTS1C.omp.def_constr(%struct.C addrspace(4)* %0)
declare spir_func void @_ZTS1C.omp.destr(%struct.C addrspace(4)* %0)
declare void @_ZTS1C.omp.copy_constr(%struct.C addrspace(4)* %0, %struct.C addrspace(4)* %1)
declare spir_func void @_ZN1CC2ERKS_(%struct.C addrspace(4)* align 4 dereferenceable_or_null(4) %this, %struct.C addrspace(4)* align 4 dereferenceable(4) %other)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2053, i32 11930794, !"_Z3foo1CS_", i32 7, i32 0, i32 0}
