; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-apply-config -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-config=%S/../Inputs/Intel_thread_limit_config.yaml -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='require<vpo-paropt-config-analysis>,function(vpo-cfg-restructuring,vpo-paropt-apply-config,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-config=%S/../Inputs/Intel_thread_limit_config.yaml -S %s | FileCheck %s

; Original code:
; void foo() {
; #pragma omp target teams thread_limit(17)
;  ;
; }

; Check that ThreadLimit(33) in Inputs/Intel_thread_limit_config.yaml
; overrides thread_limit(17) clause:
; CHECK: call i32 @__tgt_target_teams_mapper({{.*}}, i8* @__omp_offloading_805_b42e4b__Z3foo_l2.region_id,{{.*}}, i32 0, i32 33)
; CHECK: call void @__kmpc_push_num_teams({{.*}}, i32 0, i32 33)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local void @foo() {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.THREAD_LIMIT"(i32 17) ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @__tgt_register_requires(i64)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2053, i32 11808331, !"_Z3foo", i32 2, i32 0, i32 0}
