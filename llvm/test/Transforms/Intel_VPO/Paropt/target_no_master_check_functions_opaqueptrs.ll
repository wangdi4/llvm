; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S < %s | FileCheck %s

; There's a set of specific OpenCL and SPIR-V functions that can still be run on
; all threads even if they appear inside a target region but outside of any
; parallel region. This test ensures that no (new) barriers or master thread
; checks are added for these.

; CHECK-NOT: is.master.thread
; CHECK-NOT: call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NOT: is.master.thread
; CHECK-NOT: call spir_func void @_Z22__spirv_ControlBarrieriii

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

define hidden i32 @main() {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]

  %1 = call spir_func i64 @_Z13get_global_idj(i32 0)
  %2 = call spir_func i64 @_Z12get_local_idj(i32 0)
  %3 = call spir_func i64 @_Z14get_local_sizej(i32 0)
  %4 = call spir_func i64 @_Z14get_num_groupsj(i32 0)
  %5 = call spir_func i64 @_Z12get_group_idj(i32 0)
  %6 = call spir_func i64 @_Z28__spirv_GlobalInvocationId_xv()
  %7 = call spir_func i64 @_Z28__spirv_GlobalInvocationId_yv()
  %8 = call spir_func i64 @_Z28__spirv_GlobalInvocationId_zv()
  %9 = call spir_func i64 @_Z27__spirv_LocalInvocationId_xv()
  %10 = call spir_func i64 @_Z27__spirv_LocalInvocationId_yv()
  %11 = call spir_func i64 @_Z27__spirv_LocalInvocationId_zv()
  %12 = call spir_func i64 @_Z23__spirv_WorkgroupSize_xv()
  %13 = call spir_func i64 @_Z23__spirv_WorkgroupSize_yv()
  %14 = call spir_func i64 @_Z23__spirv_WorkgroupSize_zv()
  %15 = call spir_func i64 @_Z23__spirv_NumWorkgroups_xv()
  %16 = call spir_func i64 @_Z23__spirv_NumWorkgroups_yv()
  %17 = call spir_func i64 @_Z23__spirv_NumWorkgroups_zv()
  %18 = call spir_func i64 @_Z21__spirv_WorkgroupId_xv()
  %19 = call spir_func i64 @_Z21__spirv_WorkgroupId_yv()
  %20 = call spir_func i64 @_Z21__spirv_WorkgroupId_zv()
  call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
  call spir_func void @_Z9mem_fencej(i32 0)
  call spir_func void @_Z14read_mem_fencej(i32 0)
  call spir_func void @_Z15write_mem_fencej(i32 0)
  call spir_func i32 @omp_get_thread_num()
  call spir_func void @__kmpc_barrier()

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

declare spir_func i64 @_Z13get_global_idj(i32)
declare spir_func i64 @_Z12get_local_idj(i32)
declare spir_func i64 @_Z14get_local_sizej(i32)
declare spir_func i64 @_Z14get_num_groupsj(i32)
declare spir_func i64 @_Z12get_group_idj(i32)
declare spir_func i64 @_Z28__spirv_GlobalInvocationId_xv()
declare spir_func i64 @_Z28__spirv_GlobalInvocationId_yv()
declare spir_func i64 @_Z28__spirv_GlobalInvocationId_zv()
declare spir_func i64 @_Z27__spirv_LocalInvocationId_xv()
declare spir_func i64 @_Z27__spirv_LocalInvocationId_yv()
declare spir_func i64 @_Z27__spirv_LocalInvocationId_zv()
declare spir_func i64 @_Z23__spirv_WorkgroupSize_xv()
declare spir_func i64 @_Z23__spirv_WorkgroupSize_yv()
declare spir_func i64 @_Z23__spirv_WorkgroupSize_zv()
declare spir_func i64 @_Z23__spirv_NumWorkgroups_xv()
declare spir_func i64 @_Z23__spirv_NumWorkgroups_yv()
declare spir_func i64 @_Z23__spirv_NumWorkgroups_zv()
declare spir_func i64 @_Z21__spirv_WorkgroupId_xv()
declare spir_func i64 @_Z21__spirv_WorkgroupId_yv()
declare spir_func i64 @_Z21__spirv_WorkgroupId_zv()
declare spir_func void @_Z22__spirv_ControlBarrieriii(i32, i32, i32)
declare spir_func void @_Z9mem_fencej(i32)
declare spir_func void @_Z14read_mem_fencej(i32)
declare spir_func void @_Z15write_mem_fencej(i32)
declare spir_func i32 @omp_get_thread_num()
declare spir_func void @__kmpc_barrier()

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2055, i32 156313154, !"main", i32 6, i32 0, i32 0}
