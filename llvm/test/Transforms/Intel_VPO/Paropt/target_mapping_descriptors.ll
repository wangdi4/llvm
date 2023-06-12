; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -switch-to-offload -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; void foo(int x) {
; #pragma omp target map(x)
;   ;
; }

; It does not make sense to emit global data related to "omp target" mapping
; for the device compilation.
; CHECK-NOT: offload_sizes
; CHECK-NOT: offload_maptypes

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(i32 noundef %x) {
entry:
  %x.addr = alloca i32, align 4
  store i32 %x, ptr %x.addr, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr %x.addr, ptr %x.addr, i64 4, i64 3, ptr null, ptr null) ] ; MAP type: 3 = 0x3 = FROM (0x2) | TO (0x1)

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @__tgt_register_requires(i64)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 64773, i32 3825459, !"_Z3foo", i32 2, i32 0, i32 0, i32 0}
