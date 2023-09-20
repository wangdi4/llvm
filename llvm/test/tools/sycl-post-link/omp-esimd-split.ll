; Check if OpenMP SPMD-SIMD code splitting generates proper filetable
; and splits the kernels correctly.

; RUN: sycl-post-link -ompoffload-link-entries -ompoffload-explicit-simd -split-esimd -S %s -o %t.table
; RUN: FileCheck %s -input-file=%t.table --check-prefix=CHECK_TABLE
; RUN: FileCheck %s -input-file=%t_globals_0.ll --check-prefix=CHECK_GLOB
; RUN: FileCheck %s -input-file=%t_esimd_1.ll --check-prefix=CHECK_SIMD
; RUN: FileCheck %s -input-file=%t_1.ll --check-prefix=CHECK_SPMD

; CHECK_TABLE: [Code]
; CHECK_TABLE-NEXT: {{.*}}_globals_0.ll
; CHECK_TABLE-NEXT: {{.*}}_esimd_1.ll
; CHECK_TABLE-NEXT: {{.*}}_1.ll

; CHECK_GLOB: @arr = protected target_declare addrspace(1) global i32 0, align 4

; CHECK_SIMD-DAG: define weak dso_local spir_kernel void @__omp_offloading_802_2f12f8d__Z3barv_l4
; CHECK_SIMD-DAG: declare spir_func i64 @_Z27__spirv_LocalInvocationId_xv() local_unnamed_addr

; CHECK_SPMD-DAG: @arr = external protected target_declare addrspace(1) global i32, align 4
; CHECK_SPMD-DAG: define weak dso_local spir_kernel void @__omp_offloading_802_2f12f8b__Z3foov_l4
; CHECK_SPMD-DAG:   call spir_func i64 @_Z12get_local_idj(i32 0)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"

%0 = type { i32, i32, [1 x %1], i64, i64, i64 }
%1 = type { i32, i32 }

%struct.__tgt_offload_entry.0 = type { ptr addrspace(4), ptr addrspace(2), i64, i32, i32, i64 }

@arr = protected target_declare addrspace(1) global i32 0, align 4

@__omp_offloading_802_2f12f8b__Z3foov_l4_kernel_info = weak target_declare local_unnamed_addr addrspace(1) constant %0 { i32 5, i32 1, [1 x %1] [%1 { i32 0, i32 8 }], i64 0, i64 0, i64 0 }
@.omp_offloading.entry_name = internal target_declare unnamed_addr addrspace(2) constant [40 x i8] c"__omp_offloading_802_2f12f8b__Z3foov_l4\00"
@.omp_offloading.entry.__omp_offloading_802_2f12f8b__Z3foov_l4 = weak target_declare local_unnamed_addr addrspace(1) constant %struct.__tgt_offload_entry.0 { ptr addrspace(4) null, ptr addrspace(2) getelementptr inbounds ([40 x i8], ptr addrspace(2) @.omp_offloading.entry_name, i32 0, i32 0), i64 0, i32 0, i32 0, i64 40 }, section "omp_offloading_entries"
@__omp_offloading_802_2f12f8d__Z3barv_l4_kernel_info = weak target_declare local_unnamed_addr addrspace(1) constant %0 { i32 5, i32 1, [1 x %1] [%1 { i32 0, i32 8 }], i64 0, i64 0, i64 0 }
@.omp_offloading.entry_name.1 = internal target_declare unnamed_addr addrspace(2) constant [40 x i8] c"__omp_offloading_802_2f12f8d__Z3barv_l4\00"
@.omp_offloading.entry.__omp_offloading_802_2f12f8d__Z3barv_l4 = weak target_declare local_unnamed_addr addrspace(1) constant %struct.__tgt_offload_entry.0 { ptr addrspace(4) null, ptr addrspace(2) getelementptr inbounds ([40 x i8], ptr addrspace(2) @.omp_offloading.entry_name.1, i32 0, i32 0), i64 0, i32 0, i32 0, i64 40 }, section "omp_offloading_entries"

; Function Attrs: mustprogress noinline nounwind
define weak dso_local spir_kernel void @__omp_offloading_802_2f12f8b__Z3foov_l4(ptr addrspace(1) noalias %res.ascast) local_unnamed_addr {
newFuncRoot:
  %0 = tail call spir_func i64 @_Z12get_local_idj(i32 0)
  %1 = tail call spir_func i64 @_Z12get_local_idj(i32 1)
  %2 = or i64 %0, %1
  %3 = tail call spir_func i64 @_Z12get_local_idj(i32 2)
  %4 = or i64 %2, %3
  %is.master.thread = icmp eq i64 %4, 0
  tail call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
  br i1 %is.master.thread, label %DIR.OMP.TARGET.3, label %master.thread.fallthru

DIR.OMP.TARGET.3:                                 ; preds = %newFuncRoot
  store i32 2, ptr addrspace(1) %res.ascast, align 4
  store i32 3, ptr addrspace(4) addrspacecast (ptr addrspace(1) @arr to ptr addrspace(4)), align 4
  br label %master.thread.fallthru

master.thread.fallthru:                           ; preds = %newFuncRoot, %DIR.OMP.TARGET.3
  tail call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
  ret void
}

declare spir_func i64 @_Z12get_local_idj(i32) local_unnamed_addr

; Function Attrs: convergent
declare spir_func void @_Z22__spirv_ControlBarrieriii(i32, i32, i32) #1

; Function Attrs: mustprogress noinline nounwind
define weak dso_local spir_kernel void @__omp_offloading_802_2f12f8d__Z3barv_l4(ptr addrspace(1) noalias %res.ascast) local_unnamed_addr !omp_simd_kernel !0 !sycl_explicit_simd !0 !intel_reqd_sub_group_size !1 {
DIR.OMP.TARGET.2.split:
  %0 = tail call spir_func i64 @_Z27__spirv_LocalInvocationId_xv()
  %1 = tail call spir_func i64 @_Z27__spirv_LocalInvocationId_yv()
  %2 = or i64 %0, %1
  %3 = tail call spir_func i64 @_Z27__spirv_LocalInvocationId_zv()
  %4 = or i64 %2, %3
  %is.master.thread = icmp eq i64 %4, 0
  tail call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
  br i1 %is.master.thread, label %DIR.OMP.END.TARGET.4, label %DIR.OMP.END.TARGET.59.exitStub

DIR.OMP.END.TARGET.4:                             ; preds = %DIR.OMP.TARGET.2.split
  store i32 3, ptr addrspace(1) %res.ascast, align 4
  br label %DIR.OMP.END.TARGET.59.exitStub

DIR.OMP.END.TARGET.59.exitStub:                   ; preds = %DIR.OMP.END.TARGET.4, %DIR.OMP.TARGET.2.split
  tail call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
  ret void
}

declare spir_func i64 @_Z27__spirv_LocalInvocationId_xv() local_unnamed_addr

declare spir_func i64 @_Z27__spirv_LocalInvocationId_yv() local_unnamed_addr

declare spir_func i64 @_Z27__spirv_LocalInvocationId_zv() local_unnamed_addr

!0 = !{}
!1 = !{i32 1}
