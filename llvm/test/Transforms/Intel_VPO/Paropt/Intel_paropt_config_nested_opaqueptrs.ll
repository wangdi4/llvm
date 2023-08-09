; INTEL_CUSTOMIZATION
; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-apply-config -vpo-paropt-config=%S/Inputs/Intel_thread_limit_config.yaml -debug-only=vpo-paropt-apply-config -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='require<vpo-paropt-config-analysis>,function(vpo-cfg-restructuring,vpo-paropt-apply-config)' -vpo-paropt-config=%S/Inputs/Intel_thread_limit_config.yaml -debug-only=vpo-paropt-apply-config -S %s 2>&1 | FileCheck %s

; Test src:
; void foo() {
;   #pragma omp masked
;   #pragma omp target teams thread_limit(17)
;   ;
; }

; CHECK: VPO Paropt Apply Config: processing target region '__omp_offloading_10309_b1f3d9b__Z3foo_l2', index: 0
; CHECK: VPO Paropt Apply Config: config specifies ThreadLimit: '33'
; CHECK: VPO Paropt Apply Config: config overrides user-specified thread_limit: i32 17

; Check for the updated directive in the output IR
; CHECK: [ "DIR.OMP.TEAMS"(), "QUAL.OMP.THREAD_LIMIT:TYPED"(i32 33, i32 0) ]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local void @foo() {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASKED"() ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.THREAD_LIMIT"(i32 17) ]

  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.MASKED"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66313, i32 186596763, !"_Z3foo", i32 2, i32 0, i32 0}
; end INTEL_CUSTOMIZATION
