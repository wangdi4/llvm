; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing)' -S %s | FileCheck %s

; The test checks that is_device_ptr() clause is transformed into MAP+PRIVATE,
; and then the PRIVATE clause is optimized with WILOCAL, when possible.

; Original code:
; void bar1(int **);
; void bar2(int *);
; int *p;
; // CHECK-LABEL: @foo1
; // CHECK-NOT: IS_DEVICE_PTR
; // The pointer escapes, so no optimization:
; // CHECK-NOT: WILOCAL
; void foo1(int *x) {
; #pragma omp target is_device_ptr(x)
;   bar1(&x);
; }
; // CHECK-LABEL: @foo2
; // CHECK-NOT: IS_DEVICE_PTR
; // The pointer escapes, but it is privatized before that,
; // so it should be optimized:
; // CHECK: PRIVATE:WILOCAL
; void foo2(int *x) {
; #pragma omp target is_device_ptr(x)
; #pragma omp teams private(x)
;   bar1(&x);
; }
; // CHECK-LABEL: @foo3
; // CHECK-NOT: IS_DEVICE_PTR
; // No escapes, so it should be optimized:
; // CHECK: PRIVATE:WILOCAL
; void foo3(int *x) {
; #pragma omp target is_device_ptr(x)
;   bar2(x);
; }
; // CHECK-LABEL: @foo4
; // CHECK-NOT: IS_DEVICE_PTR
; // Pointer escapes inside teams, and it is not privatized,
; // so no optimization:
; // CHECK-NOT: WILOCAL
; void foo4(int *x) {
; #pragma omp target is_device_ptr(x)
; #pragma omp teams shared(x)
;   bar1(&x);
; }
; // CHECK-LABEL: @foo5
; // CHECK-NOT: IS_DEVICE_PTR
; // It should be optimized eventually, but global variables
; // are not supported by the optimization right now:
; // CHECK-NOT: WILOCAL
; void foo5(int *x) {
; #pragma omp target is_device_ptr(p)
;   bar2(p);
; }

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@p = external addrspace(1) global i32 addrspace(4)*, align 8

define hidden spir_func void @foo1(i32 addrspace(4)* %x) {
entry:
  %x.addr = alloca i32 addrspace(4)*, align 8
  %x.addr.ascast = addrspacecast i32 addrspace(4)** %x.addr to i32 addrspace(4)* addrspace(4)*
  store i32 addrspace(4)* %x, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %0 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.IS_DEVICE_PTR:PTR_TO_PTR"(i32 addrspace(4)* addrspace(4)* %x.addr.ascast) ]

  call spir_func void @bar1(i32 addrspace(4)* addrspace(4)* %x.addr.ascast)
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  ret void

; uselistorder directives
  uselistorder i32 addrspace(4)* addrspace(4)* %x.addr.ascast, { 1, 0, 2, 3 }
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare spir_func void @bar1(i32 addrspace(4)* addrspace(4)*)

define hidden spir_func void @foo2(i32 addrspace(4)* %x) {
entry:
  %x.addr = alloca i32 addrspace(4)*, align 8
  %x.addr.ascast = addrspacecast i32 addrspace(4)** %x.addr to i32 addrspace(4)* addrspace(4)*
  store i32 addrspace(4)* %x, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1),
    "QUAL.OMP.IS_DEVICE_PTR:PTR_TO_PTR"(i32 addrspace(4)* addrspace(4)* %x.addr.ascast) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* addrspace(4)* %x.addr.ascast) ]

  call spir_func void @bar1(i32 addrspace(4)* addrspace(4)* %x.addr.ascast)
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void

; uselistorder directives
  uselistorder i32 addrspace(4)* addrspace(4)* %x.addr.ascast, { 2, 1, 0, 3 }
}

define hidden spir_func void @foo3(i32 addrspace(4)* %x) {
entry:
  %x.addr = alloca i32 addrspace(4)*, align 8
  %x.addr.ascast = addrspacecast i32 addrspace(4)** %x.addr to i32 addrspace(4)* addrspace(4)*
  store i32 addrspace(4)* %x, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %0 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 2),
    "QUAL.OMP.IS_DEVICE_PTR:PTR_TO_PTR"(i32 addrspace(4)* addrspace(4)* %x.addr.ascast) ]

  %2 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  call spir_func void @bar2(i32 addrspace(4)* %2)
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  ret void

; uselistorder directives
  uselistorder i32 addrspace(4)* addrspace(4)* %x.addr.ascast, { 1, 0, 2, 3 }
}

declare spir_func void @bar2(i32 addrspace(4)*)

define hidden spir_func void @foo4(i32 addrspace(4)* %x) {
entry:
  %x.addr = alloca i32 addrspace(4)*, align 8
  %x.addr.ascast = addrspacecast i32 addrspace(4)** %x.addr to i32 addrspace(4)* addrspace(4)*
  store i32 addrspace(4)* %x, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %0 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 3),
    "QUAL.OMP.IS_DEVICE_PTR:PTR_TO_PTR"(i32 addrspace(4)* addrspace(4)* %x.addr.ascast) ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspace(4)* %x.addr.ascast) ]

  call spir_func void @bar1(i32 addrspace(4)* addrspace(4)* %x.addr.ascast)
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  ret void

; uselistorder directives
  uselistorder i32 addrspace(4)* addrspace(4)* %x.addr.ascast, { 2, 1, 0, 3, 4 }
}

define hidden spir_func void @foo5(i32 addrspace(4)* %x) {
entry:
  %x.addr = alloca i32 addrspace(4)*, align 8
  %x.addr.ascast = addrspacecast i32 addrspace(4)** %x.addr to i32 addrspace(4)* addrspace(4)*
  store i32 addrspace(4)* %x, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %0 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* addrspacecast (i32 addrspace(4)* addrspace(1)* @p to i32 addrspace(4)* addrspace(4)*), align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 4),
    "QUAL.OMP.IS_DEVICE_PTR:PTR_TO_PTR"(i32 addrspace(4)* addrspace(4)* addrspacecast (i32 addrspace(4)* addrspace(1)* @p to i32 addrspace(4)* addrspace(4)*)) ]

  %2 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* addrspacecast (i32 addrspace(4)* addrspace(1)* @p to i32 addrspace(4)* addrspace(4)*), align 8
  call spir_func void @bar2(i32 addrspace(4)* %2)
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  ret void

; uselistorder directives
  uselistorder token ()* @llvm.directive.region.entry, { 0, 2, 1, 3, 5, 4, 6 }
  uselistorder i32 addrspace(4)* addrspace(4)* addrspacecast (i32 addrspace(4)* addrspace(1)* @p to i32 addrspace(4)* addrspace(4)*), { 1, 0, 2 }
}

!omp_offload.info = !{!0, !1, !2, !3, !4}
!0 = !{i32 0, i32 2053, i32 12462031, !"_Z4foo1", i32 5, i32 0, i32 0}
!1 = !{i32 0, i32 2053, i32 12462031, !"_Z4foo2", i32 9, i32 1, i32 0}
!2 = !{i32 0, i32 2053, i32 12462031, !"_Z4foo3", i32 14, i32 2, i32 0}
!3 = !{i32 0, i32 2053, i32 12462031, !"_Z4foo4", i32 18, i32 3, i32 0}
!4 = !{i32 0, i32 2053, i32 12462031, !"_Z4foo5", i32 23, i32 4, i32 0}
