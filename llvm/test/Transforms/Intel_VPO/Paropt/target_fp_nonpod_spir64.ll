; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefix=NOCTORDTOR -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s -check-prefix=NOCTORDTOR -check-prefix=ALL
; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-emit-target-fp-ctor-dtor=true -S %s | FileCheck %s -check-prefix=CTORDTOR -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -vpo-paropt-emit-target-fp-ctor-dtor=true -S %s | FileCheck %s -check-prefix=CTORDTOR -check-prefix=ALL

; Test src:

; class C {
;   int x;
;
; public:
; #pragma omp declare target
;   C() : x(1) {}
;   ~C(){};
;   C(const C &c1) { x = c1.x; }
; #pragma omp end declare target
; };
;
; void foo() {
;   C c1;
; #pragma omp target firstprivate(c1)
;   ;
; }

; ALL-LABEL: define{{.*}}void @__omp_offloading{{[^ ]*}}foo{{[^ ]*}}(%class.C addrspace(1)* %c1.ascast)

; When -vpo-paropt-emit-target-fp-ctor-dtor is "true":
; Check that the outlined function for the target region has a copy of %c1.ascast
; created within the function, and copy-constructor and destructor are called for it.
; CTORDTOR-DAG: %[[C1_NEW:c1.ascast[^ ]*]] = alloca %class.C, align 1

; CTORDTOR-DAG: call spir_func void @_ZTS1C.omp.copy_constr(%class.C addrspace(4)* %[[DST:[^ ,]+]], %class.C addrspace(4)* %[[SRC:[^ ,)]+]]) [[ATTR1:#[0-9]+]]
; CTORDTOR-DAG: %[[SRC]] = addrspacecast %class.C addrspace(1)* %c1.ascast to %class.C addrspace(4)*
; CTORDTOR-DAG: %[[DST]] = addrspacecast %class.C* %[[C1_NEW]] to %class.C addrspace(4)*

; CTORDTOR-DAG: call spir_func void @_ZTS1C.omp.destr(%class.C addrspace(4)* %[[DST]]) [[ATTR2:#[0-9]+]]

; CTORDTOR-DAG: [[ATTR1]] = {{{.*}}"openmp-privatization-copyconstructor"{{.*}}}
; CTORDTOR-DAG: [[ATTR2]] = {{{.*}}"openmp-privatization-destructor"{{.*}}}

; When -vpo-paropt-emit-target-fp-ctor-dtor is "false":
; Check that the outlined function does not have a copy for %c1.ascast, and no
; copy-constructor/destructor call is emitted.

; NOCTORDTOR-NOT: %{{.*}} = alloca %class.C, align 1
; NOCTORDTOR-NOT: call spir_func void @_ZTS1C.omp.copy_constr({{.*}})
; NOCTORDTOR-NOT: call spir_func void @_ZTS1C.omp.destr({{.*}})

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%class.C = type { i32 }

$_ZN1CC2Ev = comdat any
$_ZN1CC2ERKS_ = comdat any
$_ZN1CD2Ev = comdat any

; Function Attrs: noinline nounwind optnone uwtable
define hidden spir_func void @_Z3foov() #0 {
entry:
  %c1 = alloca %class.C, align 4
  %c1.ascast = addrspacecast %class.C* %c1 to %class.C addrspace(4)*
  call spir_func void @_ZN1CC2Ev(%class.C addrspace(4)* %c1.ascast)

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE:NONPOD"(%class.C addrspace(4)* %c1.ascast, void (%class.C addrspace(4)*, %class.C addrspace(4)*)* @_ZTS1C.omp.copy_constr, void (%class.C addrspace(4)*)* @_ZTS1C.omp.destr), "QUAL.OMP.MAP.TO"(%class.C addrspace(4)* %c1.ascast, %class.C addrspace(4)* %c1.ascast, i64 4, i64 161) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  call spir_func void @_ZN1CD2Ev(%class.C addrspace(4)* %c1.ascast) #2
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
declare hidden spir_func void @_ZN1CC2Ev(%class.C addrspace(4)* %this) unnamed_addr #1 align 2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: noinline nounwind uwtable
declare void @_ZTS1C.omp.copy_constr(%class.C addrspace(4)* %0, %class.C addrspace(4)* %1) #3

; Function Attrs: noinline nounwind optnone uwtable
declare hidden spir_func void @_ZN1CC2ERKS_(%class.C addrspace(4)* %this, %class.C addrspace(4)* align 4 dereferenceable(4) %c1) unnamed_addr #1 align 2

; Function Attrs: noinline nounwind uwtable
declare void @_ZTS1C.omp.destr(%class.C addrspace(4)* %0) #3

; Function Attrs: noinline nounwind optnone uwtable
declare hidden spir_func void @_ZN1CD2Ev(%class.C addrspace(4)* %this) unnamed_addr #1 align 2

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { noinline nounwind uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}

!0 = !{i32 0, i32 2055, i32 151590633, !"_Z3foov", i32 14, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
