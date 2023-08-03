; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefix=NOCTORDTOR -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s -check-prefix=NOCTORDTOR -check-prefix=ALL
; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-emit-target-fp-ctor-dtor=true -S %s | FileCheck %s -check-prefix=CTORDTOR -check-prefix=ALL
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

; ALL-LABEL: define{{.*}}void @__omp_offloading{{[^ ]*}}foo{{[^ ]*}}(ptr addrspace(1) %c1.ascast)

; When -vpo-paropt-emit-target-fp-ctor-dtor is "true":
; Check that the outlined function for the target region has a copy of %c1.ascast
; created within the function, and copy-constructor and destructor are called for it.
; CTORDTOR-DAG: %[[C1_NEW:c1.ascast[^ ]*]] = alloca %class.C, align 8
; CTORDTOR-DAG: %[[SRC:[^ ,)]+]] = addrspacecast ptr addrspace(1) %c1.ascast to ptr addrspace(4)
; CTORDTOR-DAG: %[[DST:[^ ,]+]] = addrspacecast ptr %[[C1_NEW]] to ptr addrspace(4)
; CTORDTOR-DAG: call spir_func void @_ZTS1C.omp.copy_constr(ptr addrspace(4) %[[DST]], ptr addrspace(4) %[[SRC]]) [[ATTR1:#[0-9]+]]

; CTORDTOR-DAG: call spir_func void @_ZTS1C.omp.destr(ptr addrspace(4) %[[DST]]) [[ATTR2:#[0-9]+]]

; CTORDTOR-DAG: [[ATTR1]] = {{{.*}}"openmp-privatization-copyconstructor"{{.*}}}
; CTORDTOR-DAG: [[ATTR2]] = {{{.*}}"openmp-privatization-destructor"{{.*}}}

; When -vpo-paropt-emit-target-fp-ctor-dtor is "false":
; Check that the outlined function does not have a copy for %c1.ascast, and no
; copy-constructor/destructor call is emitted.

; NOCTORDTOR-NOT: %{{.*}} = alloca %class.C, align 8
; NOCTORDTOR-NOT: call spir_func void @_ZTS1C.omp.copy_constr({{.*}})
; NOCTORDTOR-NOT: call spir_func void @_ZTS1C.omp.destr({{.*}})

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%class.C = type { i32 }

define protected spir_func void @_Z3foov() {
entry:
  %c1 = alloca %class.C, align 4
  %c1.ascast = addrspacecast ptr %c1 to ptr addrspace(4)
  call spir_func void @_ZN1CC2Ev(ptr addrspace(4) noundef align 4 dereferenceable_or_null(4) %c1.ascast) #4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:NONPOD.TYPED"(ptr addrspace(4) %c1.ascast, %class.C zeroinitializer, i32 1, ptr @_ZTS1C.omp.copy_constr, ptr @_ZTS1C.omp.destr),
    "QUAL.OMP.MAP.TO"(ptr addrspace(4) %c1.ascast, ptr addrspace(4) %c1.ascast, i64 4, i64 161, ptr null, ptr null) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  call spir_func void @_ZN1CD2Ev(ptr addrspace(4) noundef align 4 dereferenceable_or_null(4) %c1.ascast) #5
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare spir_func void @_ZN1CC2Ev(ptr addrspace(4) noundef align 4 dereferenceable_or_null(4) %this)
declare void @_ZTS1C.omp.copy_constr(ptr addrspace(4) noundef %0, ptr addrspace(4) noundef %1)
declare spir_func void @_ZN1CC2ERKS_(ptr addrspace(4) noundef align 4 dereferenceable_or_null(4) %this, ptr addrspace(4) noundef align 4 dereferenceable(4) %c1)
declare spir_func void @_ZTS1C.omp.destr(ptr addrspace(4) noundef %0)
declare spir_func void @_ZN1CD2Ev(ptr addrspace(4) noundef align 4 dereferenceable_or_null(4) %this)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66312, i32 216093034, !"_Z3foov", i32 14, i32 0, i32 0, i32 0}
