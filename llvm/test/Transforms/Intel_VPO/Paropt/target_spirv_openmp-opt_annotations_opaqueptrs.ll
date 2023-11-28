; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S < %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,openmp-opt' -switch-to-offload -S < %s | FileCheck %s -check-prefix=OPENMP_OPT

; This test checks that Paropt uses the right attributes to enable barrier
; optimization in OpenMPOpt. This means annotating the function with "kernel",
; the barrier calls with "llvm.assume"="ompx_aligned_barrier", and the
; get_local_id function with all of the necessary attributes to show it doesn't
; have any side-effects.

; CHECK: define weak dso_local spir_kernel void @__omp_offloading_10302_54c1656_openmp_opt_annotations_l0() #[[#FUNC:]]

; CHECK-COUNT-3: call spir_func i64 @_Z12get_local_idj(i32 [[#]])

; CHECK-COUNT-3: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784) #[[#BARRIERS:]]

; CHECK: declare spir_func i64 @_Z12get_local_idj(i32) #[[#GET_LOCAL_ID:]]

; CHECK-DAG: attributes #[[#FUNC]] = {{.*}} "kernel"
; CHECK-DAG: attributes #[[#BARRIERS]] = {{.*}} "llvm.assume"="ompx_aligned_barrier"
; CHECK-DAG: attributes #[[#GET_LOCAL_ID]] = {{.*}} nosync nounwind willreturn memory(none)

; Thanks to these attributes, when OpenMPOpt is run it should be able to remove
; all of the barriers in this simple case.

; OPENMP_OPT-NOT: call spir_func void @_Z22__spirv_ControlBarrieriii

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define spir_func void @openmp_opt_annotations() {
  %target = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]

  call spir_func void @foo()

  %parallel = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %parallel) [ "DIR.OMP.END.PARALLEL"() ]

  call void @llvm.directive.region.exit(token %target) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare spir_func void @foo()

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}

!0 = !{i32 0, i32 66306, i32 88872534, !"openmp_opt_annotations", i32 0, i32 0, i32 0, i32 0}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"openmp-device", i32 51}
