; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original source:
; #pragma omp declare target
; static int global_x;
; #pragma omp end declare target

; Check that global_x is not externalized (i.e. internal linkage set):
; CHECK: @global_x = internal target_declare addrspace(1) global i32 0
; CHECK: @.omp_offloading.entry.global_x = weak target_declare addrspace(1) constant

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

@global_x = internal target_declare addrspace(1) global i32 0, align 4
@.global_x.ref = internal constant ptr addrspace(1) @global_x
@llvm.compiler.used = appending global [1 x ptr] [ptr @.global_x.ref], section "llvm.metadata"

!omp_offload.info = !{!0}
!0 = !{i32 1, !"global_x", i32 0, i32 0, ptr addrspace(1) @global_x}
