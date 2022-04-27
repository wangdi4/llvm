; RUN: opt %s -dpcpp-enable-direct-function-call-vectorization=true -dpcpp-kernel-sg-size-collector -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt %s -dpcpp-enable-direct-function-call-vectorization=true -dpcpp-kernel-sg-size-collector -S | FileCheck %s
; RUN: opt %s -dpcpp-enable-direct-function-call-vectorization=true -passes=dpcpp-kernel-sg-size-collector -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt %s -dpcpp-enable-direct-function-call-vectorization=true -passes=dpcpp-kernel-sg-size-collector -S | FileCheck %s

; CHECK-NOT: vector-variant
; CHECK: define internal fastcc void @_Z24calcBoxBlurFloatLocal_V2PfS_iiii
; CHECK-SAME: #0
; CHECK: attributes #0 = { nounwind "prefer-vector-width"="512" }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32-msvc-elf"

%structtype.0 = type { i32, i32 }
%structtype.4 = type { %structtype.5, i32, i32 }
%structtype.5 = type { %structtype.0, [149184 x i8] }
%struct.kmp_program_data = type { i32, i32, i32, i32, i32, i64, i64, i32 }
%struct.kmp_local_state = type { [256 x i8], %struct.kmp_thread_state addrspace(4)*, i8 addrspace(4)*, i32, i16, i16, i16, %struct.kmp_shared_data, %structtype.0, i16 }
%struct.kmp_thread_state = type { %struct.kmp_team_state, [256 x %struct.kmp_task_state], [8 x [256 x %struct.kmp_task_state]], [256 x i8 addrspace(4)*], %union.anon.2, [256 x i32], [256 x i64], [256 x i64], [256 x i64], [256 x i64], i64 }
%struct.kmp_team_state = type { %struct.kmp_task_state, %struct.kmp_task_state }
%struct.kmp_task_state = type { %struct.anon, %struct.anon.0, %struct.kmp_task_state addrspace(4)* }
%struct.anon = type { i64, i64, i64, i64, i32 }
%struct.anon.0 = type { i8, i8, i16, i16, i16, i16, i64 }
%union.anon.2 = type { [256 x i16] }
%struct.kmp_shared_data = type { [64 x i8 addrspace(4)*], i32 }
%struct.__tgt_offload_entry = type { i8 addrspace(4)*, i8 addrspace(2)*, i64, i32, i32, i64 }

@.omp_offloading.entry_name = internal unnamed_addr addrspace(2) constant [79 x i8] c"__omp_offloading_f66006ff_1a664__ZN17BoxBlurFloatLocal15execute_offloadEi_l565\00"
@__omp_spirv_global_data = local_unnamed_addr addrspace(1) global %structtype.4 { %structtype.5 { %structtype.0 zeroinitializer, [149184 x i8] undef }, i32 1, i32 0 }, align 8
@__omp_spirv_program_data = local_unnamed_addr addrspace(1) global %struct.kmp_program_data { i32 0, i32 0, i32 -1, i32 0, i32 0, i64 0, i64 0, i32 0 }, align 8
@__omp_spirv_spmd_num_threads = local_unnamed_addr addrspace(1) global [1048576 x i16] zeroinitializer, align 2
@__omp_spirv_local_data = local_unnamed_addr addrspace(1) global [504 x %struct.kmp_local_state] zeroinitializer, align 8
@__omp_spirv_thread_data = local_unnamed_addr addrspace(1) global [504 x %struct.kmp_thread_state] zeroinitializer, align 8
@__omp_offloading_entries_table = local_unnamed_addr addrspace(2) constant [1 x %struct.__tgt_offload_entry] [%struct.__tgt_offload_entry { i8 addrspace(4)* null, i8 addrspace(2)* getelementptr inbounds ([79 x i8], [79 x i8] addrspace(2)* @.omp_offloading.entry_name, i32 0, i32 0), i64 0, i32 0, i32 0, i64 74 }]
@__omp_offloading_entries_table_size = local_unnamed_addr addrspace(2) constant i64 40

; Function Attrs: nounwind
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #0

; Function Attrs: nounwind
define internal fastcc void @_Z24calcBoxBlurFloatLocal_V2PfS_iiii(float addrspace(4)* nocapture %0, float addrspace(4)* nocapture readonly %1, i32 %2, i32 %3, i32 %4) #0 !recommended_vector_length !1 {
  ret void
}

; Function Attrs: nounwind
define void @__omp_offloading_f66006ff_1a664__ZN17BoxBlurFloatLocal15execute_offloadEi_l565(float addrspace(1)* %0, float addrspace(1)* %1, i64 %2, i64 %3, i64 %4, i64 %5, i64 %6) #0 !recommended_vector_length !1 {
  %8 = addrspacecast float addrspace(1)* %0 to float addrspace(4)*
  %9 = addrspacecast float addrspace(1)* %1 to float addrspace(4)*
  %10 = trunc i64 %3 to i32
  %11 = trunc i64 %2 to i32
  %12 = tail call i64 @_Z13get_global_idj(i32 0)
  %13 = trunc i64 %12 to i32
  %14 = shl nsw i32 %13, 2
  tail call fastcc void @_Z24calcBoxBlurFloatLocal_V2PfS_iiii(float addrspace(4)* nocapture %8, float addrspace(4)* nocapture readonly %9, i32 %10, i32 %11, i32 %14) #3
  ret void
}


attributes #0 = { nounwind "prefer-vector-width"="512" }

!sycl.kernels = !{!0}
!spirv.Source = !{!2}

!0 = !{void (float addrspace(1)*, float addrspace(1)*, i64, i64, i64, i64, i64)* @__omp_offloading_f66006ff_1a664__ZN17BoxBlurFloatLocal15execute_offloadEi_l565}
!1 = !{i32 16}
!2 = !{i32 4, i32 200000}

; DEBUGIFY-NOT: WARNING
