; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test that the flags -vpo-paropt-disable-offload
;   and               -vpo-paropt-prepare-disable-offload
; remove the TARGET [*] constructs from the program, while
; still handling the other OMP constructs.
;
; Original C test is:
;
;  #include <stdio.h>
;  int main() {
;    int x;
;    #pragma omp target parallel map(x)
;      x = 123;
;    return x;
;  }
;
; When the flags are used, the "target" part of the construct is ignored,
; resulting in
;    #pragma omp parallel
;
; Verify that offloading code was not emitted
; CHECK-NOT: call{{.*}}@__tgt_target
;
; Verify that other OMP code unrelated to offloading was emitted
; CHECK: call void {{.*}} @__kmpc_fork_call({{.*}})

; ModuleID = 'target_no_offload.c'
source_filename = "target_no_offload.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr %x, ptr %x, i64 4, i64 35, ptr null, ptr null) ] ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %x, i32 0, i32 1) ]

  store i32 123, ptr %x, align 4
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  %2 = load i32, ptr %x, align 4
  ret i32 %2
}

declare token @llvm.directive.region.entry() 

declare void @llvm.directive.region.exit(token) 

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 64773, i32 3825459, !"_Z4main", i32 4, i32 0, i32 0, i32 0}
