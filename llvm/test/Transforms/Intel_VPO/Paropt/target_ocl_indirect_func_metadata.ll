; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
;void indirect_func(void) {}
;int VAR;
;
;#pragma omp declare target to(VAR)
;#pragma omp declare target to(indirect_func) //indirect(true)

; The offload metadata for the indirect function was added manually.

; CHECK: [[NAME:@.omp_offloading.entry_name.*]] = internal target_declare unnamed_addr addrspace(2) constant [19 x i8] c"_Z13indirect_funcv\00"
; CHECK: @.omp_offloading.entry._Z13indirect_funcv = weak target_declare addrspace(1) constant %struct.__tgt_offload_entry { ptr addrspace(4) addrspacecast (ptr @indirect_func to ptr addrspace(4)), ptr addrspace(2) [[NAME]], i64 0, i32 8, i32 0, i64 19 }, section "omp_offloading_entries"

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@VAR = hidden target_declare addrspace(1) global i32 0, align 4

define hidden spir_func void @indirect_func() #0 {
entry:
  ret void
}

attributes #0 = { "openmp-target-declare"="true" }

!omp_offload.info = !{!0,!1}
!0 = !{i32 1, !"_Z3VAR", i32 0, i32 0, ptr addrspace(1) @VAR}
!1 = !{i32 2, !"_Z13indirect_funcv", i32 1, ptr @indirect_func}
