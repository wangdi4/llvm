; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt -S -vpo-paropt-atomic-free-reduction-par-global=false -vpo-paropt-atomic-free-red-global-combiner-selector=last-team-inlined <%s | FileCheck -check-prefix=LASTTEAMINLINED %s
; RUN: opt -switch-to-offload -passes="function(vpo-cfg-restructuring),vpo-paropt" -S -vpo-paropt-atomic-free-reduction-par-global=false -vpo-paropt-atomic-free-red-global-combiner-selector=last-team-inlined <%s | FileCheck -check-prefix=LASTTEAMINLINED %s

; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt -S -vpo-paropt-atomic-free-reduction-par-global=false -vpo-paropt-atomic-free-red-global-combiner-selector=team-zero-rtl <%s | FileCheck -check-prefix=TEAMZERORTL %s
; RUN: opt -switch-to-offload -passes="function(vpo-cfg-restructuring),vpo-paropt" -S -vpo-paropt-atomic-free-reduction-par-global=false -vpo-paropt-atomic-free-red-global-combiner-selector=team-zero-rtl <%s | FileCheck -check-prefix=TEAMZERORTL %s

; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt -S -vpo-paropt-atomic-free-reduction-par-global=false -vpo-paropt-atomic-free-red-global-combiner-selector=last-team-rtl <%s | FileCheck -check-prefix=LASTTEAMRTL %s
; RUN: opt -switch-to-offload -passes="function(vpo-cfg-restructuring),vpo-paropt" -S -vpo-paropt-atomic-free-reduction-par-global=false -vpo-paropt-atomic-free-red-global-combiner-selector=last-team-rtl <%s | FileCheck -check-prefix=LASTTEAMRTL %s

; Test src:
;
; int b = 0;
;
; void foo() {
;   #pragma omp target teams reduction(+:b)
;     b = 111;
; }

; The test IR is a reduced version of the IR after vpo-restore-operands pass,
; for the above test.

; Check that we properly populated the teams reduction API call for combining
; the final teams reduction buffers.


; With LASTTEAMINLINED - we expect compiler generated last team will combine the teams partial results

; LASTTEAMINLINED: master.thread.code{{.*}}:

; LASTTEAMINLINED: counter_check:
; LASTTEAMINLINED: %[[NUM_GROUPS:[^,]+]] = call spir_func i64 @_Z14get_num_groupsj(i32 0)
; LASTTEAMINLINED: %[[TEAMS_COUNTER:[^,]+]] = addrspacecast ptr addrspace(1) %[[TEAMS_COUNTER_PTR:[^,]+]] to ptr addrspace(4)
; LASTTEAMINLINED: %[[UPD_CNTR:[^,]+]] = call spir_func i32 @__kmpc_atomic_fixed4_add_cpt(ptr addrspace(4) %[[TEAMS_COUNTER]], i32 1, i32 1)
; LASTTEAMINLINED: %[[NUM_GROUPS_TRUNC:[^,]+]] = trunc i64 %[[NUM_GROUPS]] to i32
; LASTTEAMINLINED: %[[CNTR_CHECK:[^,]+]] = icmp ne i32 %[[UPD_CNTR]], %[[NUM_GROUPS_TRUNC]]
; LASTTEAMINLINED: br i1 %[[CNTR_CHECK]], label %{{.*}}, label %team.red.buffers.ready

; LASTTEAMINLINED: master.thread.fallthru{{.*}}:

; With TEAMZERORTL - we expect a call to __kmpc_team_reduction_ready_teamzero(..)

; TEAMZERORTL: call spir_func void @_Z{{.*}}__spirv_ControlBarrieriii(i32 {{.*}}, i32 {{.*}}, i32 {{.*}})
; TEAMZERORTL: br i1 %is.master.thread{{.*}}, label %master.thread.code{{.*}}, label %master.thread.fallthru{{.*}}
; TEAMZERORTL: master.thread.code{{[0-9]*}}:

; TEAMZERORTL: counter_check:
; TEAMZERORTL: %[[NUM_GROUPS:[^,]+]] = call spir_func i64 @_Z14get_num_groupsj(i32 0)
; TEAMZERORTL: %[[NUM_GROUPS_TRUNC:[^,]+]] = trunc i64 %[[NUM_GROUPS]] to i32
; TEAMZERORTL: %[[TEAMS_COUNTER:[^,]+]] = addrspacecast ptr addrspace(1) %[[TEAMS_COUNTER_PTR:[^,]+]] to ptr addrspace(4)
; TEAMZERORTL: %[[UPD_CNTR:[^,]+]] = call spir_func i1 @__kmpc_team_reduction_ready_teamzero(ptr addrspace(4) %[[TEAMS_COUNTER]], i32 %[[NUM_GROUPS_TRUNC]])
; TEAMZERORTL: %[[CNTR_CHECK:[^,]+]] = icmp ne i1 %[[UPD_CNTR]], true
; TEAMZERORTL: br i1 %[[CNTR_CHECK]], label %{{.*}}, label %team.red.buffers.ready

; TEAMZERORTL: br label %master.thread.fallthru{{.*}}
; TEAMZERORTL: master.thread.fallthru{{[0-9]*}}:
; TEAMZERORTL: call spir_func void @_Z{{.*}}__spirv_ControlBarrieriii(i32 {{.*}}, i32 {{.*}}, i32 {{.*}})

; With LASTTEAMRTL - we expect a call to __kmpc_team_reduction_ready(..)

; LASTTEAMRTL: call spir_func void @_Z{{.*}}__spirv_ControlBarrieriii(i32 {{.*}}, i32 {{.*}}, i32 {{.*}})
; LASTTEAMRTL: br i1 %is.master.thread{{.*}}, label %master.thread.code{{.*}}, label %master.thread.fallthru{{.*}}
; LASTTEAMRTL: master.thread.code{{[0-9]*}}:

; LASTTEAMRTL: counter_check:
; LASTTEAMRTL: %[[NUM_GROUPS:[^,]+]] = call spir_func i64 @_Z14get_num_groupsj(i32 0)
; LASTTEAMRTL: %[[NUM_GROUPS_TRUNC:[^,]+]] = trunc i64 %[[NUM_GROUPS]] to i32
; LASTTEAMRTL: %[[TEAMS_COUNTER:[^,]+]] = addrspacecast ptr addrspace(1) %[[TEAMS_COUNTER_PTR:[^,]+]] to ptr addrspace(4)
; LASTTEAMRTL: %[[UPD_CNTR:[^,]+]] = call spir_func i1 @__kmpc_team_reduction_ready(ptr addrspace(4) %[[TEAMS_COUNTER]], i32 %[[NUM_GROUPS_TRUNC]])
; LASTTEAMRTL: %[[CNTR_CHECK:[^,]+]] = icmp ne i1 %[[UPD_CNTR]], true
; LASTTEAMRTL: br i1 %[[CNTR_CHECK]], label %{{.*}}, label %team.red.buffers.ready

; LASTTEAMRTL: br label %master.thread.fallthru{{.*}}
; LASTTEAMRTL: master.thread.fallthru{{[0-9]*}}:
; LASTTEAMRTL: call spir_func void @_Z{{.*}}__spirv_ControlBarrieriii(i32 {{.*}}, i32 {{.*}}, i32 {{.*}})

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@b = external addrspace(1) global i32

define spir_func void @widget() {
bb:
  br label %bb1

bb1:                                              ; preds = %bb
  %tmp = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  br label %bb2

bb2:                                              ; preds = %bb1
  %tmp3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(1) @b, i32 0, i32 1) ]

  store i32 111, ptr addrspace(4) addrspacecast (ptr addrspace(1) @b to ptr addrspace(4)), align 4
  br label %bb4

bb4:                                              ; preds = %bb2
  call void @llvm.directive.region.exit(token %tmp3) [ "DIR.OMP.END.TEAMS"() ]
  br label %bb5

bb5:                                              ; preds = %bb4
  call void @llvm.directive.region.exit(token %tmp) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66313, i32 46789301, !"_Z3foo", i32 4, i32 0, i32 0, i32 0}
