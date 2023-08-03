; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-paropt-prepare -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-paropt-prepare),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; #pragma omp declare target
; int X;
; #pragma omp end declare target

; CHECK-DAG: %struct.__tgt_offload_entry = type { ptr addrspace(4), ptr addrspace(2), i64, i32, i32, i64 }
; CHECK-DAG: @X = protected target_declare addrspace(1) global i32 0, align 4
; CHECK-DAG: @.omp_offloading.entry_name = internal target_declare unnamed_addr addrspace(2) constant [5 x i8] c"_Z1X\00"
; CHECK-DAG: @.omp_offloading.entry._Z1X = weak target_declare addrspace(1) constant %struct.__tgt_offload_entry { ptr addrspace(4) addrspacecast (ptr addrspace(1) @X to ptr addrspace(4)), ptr addrspace(2) @.omp_offloading.entry_name, i64 4, i32 0, i32 0, i64 5 }, section "omp_offloading_entries"

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@X = protected target_declare addrspace(1) global i32 0, align 4

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 1, !"_Z1X", i32 0, i32 0, ptr addrspace(1) @X}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 8, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
