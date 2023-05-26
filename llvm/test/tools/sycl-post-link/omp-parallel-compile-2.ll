; This test checks that the post-link tool generates a table file with IR files
; for OpenMP offload if parallel compilation is executed.
; First file in the table should contain externalized global variables.
; Each other IR file should contain a kernel functions with dependencies.

; RUN: sycl-post-link -ompoffload-link-entries -split=kernel -S %s -o %t.table
; RUN: FileCheck %s -input-file=%t.table --check-prefix=CHECK-TABLE
; RUN: FileCheck %s -input-file=%t_globals_0.ll --check-prefix=CHECK-GLOB
; RUN: FileCheck %s -input-file=%t_1.ll --check-prefix=CHECK-KERN1

; CHECK-TABLE: [Code]
; CHECK-TABLE-NEXT: {{.*}}_globals_0.ll
; CHECK-TABLE-NEXT: {{.*}}_1.ll

; CHECK-GLOB: @__omp_offloading_{{.*}}__Z4main_{{.*}}_kernel_info = weak target_declare local_unnamed_addr addrspace(1) constant
; CHECK-GLOB-NEXT: @.omp_offloading.entry_name = dso_local unnamed_addr addrspace(2) constant [41 x i8] c"__omp_offloading_fd05_172e0a1__Z4main_l6\00"
; CHECK-GLOB-NEXT: @__omp_offloading_entries_table = addrspace(1) constant [1 x %struct.__tgt_offload_entry]
; CHECK-GLOB-NEXT: @__omp_offloading_entries_table_size = addrspace(1) constant i64


; CHECK-KERN1-DAG: @.str = private unnamed_addr addrspace(3) constant [7 x i8] c"Hello\0A\00", align 1
; CHECK-KERN1-DAG: @.str.2 = internal unnamed_addr addrspace(3) constant [7 x i8] c"World\0A\00", align 1

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"

%0 = type { i32, i32, i64, i64, i64 }
%struct.__tgt_offload_entry = type { i8 addrspace(4)*, i8 addrspace(2)*, i64, i32, i32, i64 }

@.str = private unnamed_addr addrspace(3) constant [7 x i8] c"Hello\0A\00", align 1
@.str.2 = internal unnamed_addr addrspace(3) constant [7 x i8] c"World\0A\00", align 1
@__omp_offloading_fd05_172e0a1__Z4main_l6_kernel_info = weak target_declare local_unnamed_addr addrspace(1) constant %0 { i32 5, i32 0, i64 0, i64 0, i64 0 }
@.omp_offloading.entry_name = internal unnamed_addr addrspace(2) constant [41 x i8] c"__omp_offloading_fd05_172e0a1__Z4main_l6\00"
@.omp_offloading.entry.__omp_offloading_fd05_172e0a1__Z4main_l6 = weak target_declare local_unnamed_addr addrspace(1) constant %struct.__tgt_offload_entry { i8 addrspace(4)* null, i8 addrspace(2)* getelementptr inbounds ([41 x i8], [41 x i8] addrspace(2)* @.omp_offloading.entry_name, i32 0, i32 0), i64 0, i32 0, i32 0, i64 41 }, section "omp_offloading_entries"

declare spir_func i32 @_Z18__spirv_ocl_printfPU3AS2cz(i8 addrspace(3)*, ...) local_unnamed_addr

; Function Attrs: mustprogress noinline norecurse nounwind
define weak dso_local spir_kernel void @__omp_offloading_fd05_172e0a1__Z4main_l6() local_unnamed_addr {
newFuncRoot:
  %0 = tail call spir_func i64 @_Z12get_local_idj(i32 0)
  %1 = tail call spir_func i64 @_Z12get_local_idj(i32 1)
  %2 = or i64 %0, %1
  %3 = tail call spir_func i64 @_Z12get_local_idj(i32 2)
  %4 = or i64 %2, %3
  %is.master.thread = icmp eq i64 %4, 0
  tail call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
  br i1 %is.master.thread, label %master.thread.code, label %master.thread.fallthru

master.thread.code:                               ; preds = %newFuncRoot
  %5 = tail call i32 (i8 addrspace(3)*, ...) @_Z18__spirv_ocl_printfPU3AS2cz(i8 addrspace(3)* getelementptr inbounds ([7 x i8], [7 x i8] addrspace(3)* @.str, i64 0, i64 0))
  %6 = tail call i32 (i8 addrspace(3)*, ...) @_Z18__spirv_ocl_printfPU3AS2cz(i8 addrspace(3)* getelementptr inbounds ([7 x i8], [7 x i8] addrspace(3)* @.str.2, i64 0, i64 0))
  br label %master.thread.fallthru

master.thread.fallthru:                           ; preds = %newFuncRoot, %master.thread.code
  tail call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
  ret void
}

declare spir_func i64 @_Z12get_local_idj(i32) local_unnamed_addr

; Function Attrs: convergent
declare spir_func void @_Z22__spirv_ControlBarrieriii(i32, i32, i32)
