; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
;void indirect_func(void) {}
;int VAR;
;
;#pragma omp declare target to(VAR)
;#pragma omp declare target to(indirect_func) //indirect(true)

; The offload metadata for the indirect function was added manually.

; CHECK: [[NAME:@.omp_offloading.entry_name.*]] = internal target_declare unnamed_addr addrspace(2) constant [19 x i8] c"_Z13indirect_funcv\00"
; CHECK: @.omp_offloading.entry._Z13indirect_funcv = weak target_declare addrspace(1) constant %struct.__tgt_offload_entry { i8 addrspace(4)* addrspacecast (i8* bitcast (void ()* @indirect_func to i8*) to i8 addrspace(4)*), i8 addrspace(2)* getelementptr inbounds ([19 x i8], [19 x i8] addrspace(2)* [[NAME]], i32 0, i32 0), i64 0, i32 8, i32 0, i64 19 }, section "omp_offloading_entries"

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
!0 = !{i32 1, !"_Z3VAR", i32 0, i32 0, i32 addrspace(1)* @VAR}
!1 = !{i32 2, !"_Z13indirect_funcv", i32 1, void ()* @indirect_func}
