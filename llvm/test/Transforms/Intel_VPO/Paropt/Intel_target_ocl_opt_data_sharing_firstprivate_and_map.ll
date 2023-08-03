; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing)' -S %s | FileCheck %s

; Original code:
;class C {
;public:
;  int *_p;
;
;  void foo(int *p) {
;#pragma omp target
;    _p = p;
;  }
;};
;
;C c;
;
;extern void bar() {
;  int x;
;  c.foo(&x);
;}

; Check that we do not set WILOCAL for the FIRSTPRIVATE this pointer.
; FIRSTPRIVATE is misleading here, because this pointer is mapped
; with the map type that conflicts with FIRSTPRIVATE mapping.
; CHECK: QUAL.OMP.PRIVATE:TYPED.WILOCAL
; CHECK-NOT: QUAL.OMP.FIRSTPRIVATE:TYPED.WILOCAL

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%class.C = type { ptr addrspace(4) }

$_ZN1C3fooEPi = comdat any

define linkonce_odr hidden spir_func void @_ZN1C3fooEPi(ptr addrspace(4) align 8 dereferenceable_or_null(8) %this, ptr addrspace(4) %p) comdat align 2 {
entry:
  %this.addr = alloca ptr addrspace(4), align 8
  %this.addr.ascast = addrspacecast ptr %this.addr to ptr addrspace(4)
  %p.addr = alloca ptr addrspace(4), align 8
  %p.addr.ascast = addrspacecast ptr %p.addr to ptr addrspace(4)
  %p.map.ptr.tmp = alloca ptr addrspace(4), align 8
  %p.map.ptr.tmp.ascast = addrspacecast ptr %p.map.ptr.tmp to ptr addrspace(4)
  store ptr addrspace(4) %this, ptr addrspace(4) %this.addr.ascast, align 8
  store ptr addrspace(4) %p, ptr addrspace(4) %p.addr.ascast, align 8
  %this1 = load ptr addrspace(4), ptr addrspace(4) %this.addr.ascast, align 8
  %0 = load ptr addrspace(4), ptr addrspace(4) %p.addr.ascast, align 8

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %this1, ptr addrspace(4) %this1, i64 8, i64 547, ptr null, ptr null), ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %0, ptr addrspace(4) %0, i64 0, i64 544, ptr null, ptr null), ; MAP type: 544 = 0x220 = IMPLICIT (0x200) | TARGET_PARAM (0x20)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %p.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %this1, %class.C zeroinitializer, i32 1) ]

  store ptr addrspace(4) %0, ptr addrspace(4) %p.map.ptr.tmp.ascast, align 8
  %2 = load ptr addrspace(4), ptr addrspace(4) %p.map.ptr.tmp.ascast, align 8
  store ptr addrspace(4) %2, ptr addrspace(4) %this1, align 8
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2053, i32 12460685, !"_ZN1C3fooEPi", i32 6, i32 0, i32 0}
