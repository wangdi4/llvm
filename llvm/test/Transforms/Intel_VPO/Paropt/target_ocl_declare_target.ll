; RUN: opt -switch-to-offload -vpo-paropt-prepare -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-paropt-prepare),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; #pragma omp declare target
; int X;
; #pragma omp end declare target

; CHECK-DAG: %struct.__tgt_offload_entry = type { i8 addrspace(4)*, i8 addrspace(2)*, i64, i32, i32, i64 }
; CHECK-DAG: @X = hidden target_declare addrspace(1) global i32 0, align 4
; CHECK-DAG: @.omp_offloading.entry_name{{.*}} = internal target_declare unnamed_addr addrspace(2) constant [2 x i8] c"X\00"
; CHECK-DAG: @.omp_offloading.entry.X = weak target_declare addrspace(1) constant %struct.__tgt_offload_entry { i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast (i32 addrspace(1)* @X to i8 addrspace(1)*) to i8 addrspace(4)*), i8 addrspace(2)* getelementptr inbounds ([2 x i8], [2 x i8] addrspace(2)* @.omp_offloading.entry_name{{.*}}, i32 0, i32 0), i64 4, i32 0, i32 0, i64 2 }, section "omp_offloading_entries"

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

@X = hidden target_declare addrspace(1) global i32 0, align 4

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"X", i32 0, i32 0, i32 addrspace(1)* @X}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 9.0.0"}
