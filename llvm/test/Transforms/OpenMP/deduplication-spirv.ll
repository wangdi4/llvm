; INTEL_COLLAB
; RUN: opt -openmp-opt-cgscc -S < %s | FileCheck %s
; RUN: opt -passes=openmp-opt-cgscc -S < %s | FileCheck %s
;
; This test checks that openmp-opt pass does not deduplicate runtime calls
; in spirv kernels.
;
; Original test source:
; int main() {
; #pragma omp target
;   {
;     printf("threads in target: %d\n", omp_get_num_threads());
; #pragma omp parallel
;     printf("threads in parallel: %d\n", omp_get_num_threads());
;   }
;   return 0;
; }
;
; CHECK-LABEL: void @__omp_offloading_54_d6771c2a__Z4main_l6(
; CHECK-COUNT-2: call spir_func i32 @omp_get_num_threads()

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%0 = type { i32, i32, i64, i64, i64 }
%struct.__tgt_offload_entry = type { i8 addrspace(4)*, i8 addrspace(2)*, i64, i32, i32, i64 }

@.str.as2 = private target_declare addrspace(2) constant [23 x i8] c"threads in target: %d\0A\00"
@.str.1.as2 = private target_declare addrspace(2) constant [25 x i8] c"threads in parallel: %d\0A\00"
@__omp_offloading_54_d6771c2a__Z4main_l6_kernel_info = weak target_declare local_unnamed_addr addrspace(1) constant %0 { i32 4, i32 0, i64 0, i64 0, i64 0 }
@.omp_offloading.entry_name = internal target_declare unnamed_addr addrspace(2) constant [40 x i8] c"__omp_offloading_54_d6771c2a__Z4main_l6\00"
@.omp_offloading.entry.__omp_offloading_54_d6771c2a__Z4main_l6 = weak target_declare local_unnamed_addr addrspace(1) constant %struct.__tgt_offload_entry { i8 addrspace(4)* null, i8 addrspace(2)* getelementptr inbounds ([40 x i8], [40 x i8] addrspace(2)* @.omp_offloading.entry_name, i32 0, i32 0), i64 0, i32 0, i32 0, i64 40 }, section "omp_offloading_entries"

define weak dso_local spir_kernel void @__omp_offloading_54_d6771c2a__Z4main_l6() {
newFuncRoot:
  %0 = call spir_func i64 @_Z12get_local_idj(i32 0)
  %1 = icmp eq i64 %0, 0
  %2 = call spir_func i64 @_Z12get_local_idj(i32 1)
  %3 = icmp eq i64 %2, 0
  %4 = and i1 %1, %3
  %5 = call spir_func i64 @_Z12get_local_idj(i32 2)
  %6 = icmp eq i64 %5, 0
  %is.master.thread = and i1 %4, %6
  call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
  br i1 %is.master.thread, label %master.thread.code, label %master.thread.fallthru

master.thread.code:                               ; preds = %newFuncRoot
  call spir_func void @__kmpc_spmd_push_num_threads(i32 1)
  br label %master.thread.fallthru

master.thread.fallthru:                           ; preds = %newFuncRoot, %master.thread.code
  call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
  %call = call spir_func i32 @omp_get_num_threads()
  call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
  br i1 %is.master.thread, label %master.thread.code1, label %master.thread.fallthru2

master.thread.code1:                              ; preds = %master.thread.fallthru
  %oclPrint14 = call i32 (i8 addrspace(2)*, ...) @_Z18__spirv_ocl_printfPU3AS2ci(i8 addrspace(2)* getelementptr inbounds ([23 x i8], [23 x i8] addrspace(2)* @.str.as2, i64 0, i64 0), i32 %call)
  call spir_func void @__kmpc_spmd_pop_num_threads()
  br label %master.thread.fallthru2

master.thread.fallthru2:                          ; preds = %master.thread.fallthru, %master.thread.code1
  call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
  br label %DIR.OMP.PARALLEL.6

DIR.OMP.PARALLEL.6:                               ; preds = %master.thread.fallthru2
  %call2 = call spir_func i32 @omp_get_num_threads()
  %oclPrint = call i32 (i8 addrspace(2)*, ...) @_Z18__spirv_ocl_printfPU3AS2ci(i8 addrspace(2)* getelementptr inbounds ([25 x i8], [25 x i8] addrspace(2)* @.str.1.as2, i64 0, i64 0), i32 %call2)
  call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
  call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
  br i1 %is.master.thread, label %master.thread.code3, label %master.thread.fallthru4

master.thread.code3:                              ; preds = %DIR.OMP.PARALLEL.6
  call spir_func void @__kmpc_spmd_push_num_threads(i32 1)
  call spir_func void @__kmpc_spmd_pop_num_threads()
  br label %master.thread.fallthru4

master.thread.fallthru4:                          ; preds = %DIR.OMP.PARALLEL.6, %master.thread.code3
  call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
  ret void
}

declare spir_func i32 @omp_get_num_threads() local_unnamed_addr

declare spir_func void @__kmpc_spmd_push_num_threads(i32) local_unnamed_addr

declare spir_func void @__kmpc_spmd_pop_num_threads() local_unnamed_addr

declare spir_func i64 @_Z12get_local_idj(i32) local_unnamed_addr

declare spir_func void @_Z22__spirv_ControlBarrieriii(i32, i32, i32)

declare spir_func noundef i32 @_Z18__spirv_ocl_printfPU3AS2ci(i8 addrspace(2)* nocapture noundef readonly, ...) local_unnamed_addr

!llvm.module.flags = !{!0, !1}

!0 = !{i32 7, !"openmp", i32 50}
!1 = !{i32 7, !"openmp-device", i32 50}
; end INTEL_COLLAB
