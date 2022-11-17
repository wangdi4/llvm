; RUN: opt -dpcpp-kernel-prepare-args -S %s | FileCheck %s
; RUN: opt -passes='dpcpp-kernel-prepare-args' -S %s | FileCheck %s

; RUN: opt -dpcpp-kernel-prepare-args -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='dpcpp-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; This test checks that block_invoke kernel function is not broken by
; PrepareKernelArgs pass.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct.ndrange_t.6 = type { i32, [3 x i64], [3 x i64], [3 x i64] }
%opencl.queue_t.5 = type opaque

define internal void @__block_fn_block_invoke(i8 addrspace(4)* %.block_descriptor, i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* noalias %pWorkDim, i64* noalias %pWGId, [4 x i64] %BaseGlbId, i8* noalias %pSpecialBuf, {}* noalias %RuntimeHandle) #0 {
entry:
  %0 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i64 0, i32 5
  %RuntimeInterface4 = load {}*, {}** %0, align 1
  %1 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i64 0, i32 6
  %2 = bitcast {}** %1 to i8**
  %Block2KernelMapper5 = load i8*, i8** %2, align 1
  %tmp.i2 = alloca %struct.ndrange_t.6, align 8
  %block.i = alloca <{ i32, i32, i8 addrspace(4)*, i64, i64 }>, align 8
  %3 = bitcast <{ i32, i32, i8 addrspace(4)*, i64, i64 }>* %block.i to i8*
  %4 = bitcast {}* %RuntimeInterface4 to i8*
  %5 = addrspacecast i8* %4 to i8 addrspace(4)*
  %call1.i = tail call %opencl.queue_t.5* @ocl20_get_default_queue(i8 addrspace(4)* %5) #4
  %block.invoke.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i64, i64 }>, <{ i32, i32, i8 addrspace(4)*, i64, i64 }>* %block.i, i64 0, i32 2
  store i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8 addrspace(4)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*, i64*, [4 x i64], i8*, {}*)* @__block_fn_block_invoke to i8*) to i8 addrspace(4)*), i8 addrspace(4)** %block.invoke.i, align 8
  %6 = addrspacecast i8* %3 to i8 addrspace(4)*
  %7 = bitcast {}* %RuntimeInterface4 to i8*
  %8 = addrspacecast i8* %7 to i8 addrspace(4)*
  %9 = addrspacecast i8* %Block2KernelMapper5 to i8 addrspace(4)*
  %10 = bitcast {}* %RuntimeHandle to i8*
  %11 = addrspacecast i8* %10 to i8 addrspace(4)*
  %ndrange.ascast.i = addrspacecast %struct.ndrange_t.6* %tmp.i2 to %struct.ndrange_t.6 addrspace(4)*

; CHECK-LABEL: @__block_fn_block_invoke(
; CHECK: call i32 @__ocl20_enqueue_kernel_basic
; CHECK-SAME: i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8*, i64*, {}*)* @__block_fn_block_invoke_kernel to i8*) to i8 addrspace(4)*)

  %call2.i = call i32 @__ocl20_enqueue_kernel_basic(%opencl.queue_t.5* %call1.i, i32 1, %struct.ndrange_t.6 addrspace(4)* %ndrange.ascast.i, i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8 addrspace(4)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*, i64*, [4 x i64], i8*, {}*)* @__block_fn_block_invoke_kernel to i8*) to i8 addrspace(4)*), i8 addrspace(4)* %6, i8 addrspace(4)* %8, i8 addrspace(4)* %9, i8 addrspace(4)* %11) #5

  ret void
}

; CHECK: define dso_local void @____block_fn_block_invoke_kernel_separated_args

define dso_local void @__block_fn_block_invoke_kernel(i8 addrspace(4)* %0, i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* noalias %pWorkDim, i64* noalias %pWGId, [4 x i64] %BaseGlbId, i8* noalias %pSpecialBuf, {}* noalias %RuntimeHandle) #1 !no_barrier_path !1 !kernel_has_sub_groups !1 !block_literal_size !2 !local_buffer_size !3 !barrier_buffer_size !4 !kernel_execution_length !5 !kernel_has_barrier !1 !kernel_has_global_sync !1 !private_memory_size !6 !opencl.stats.InstCounter.CantVectNonInlineUnsupportedFunctions !7 {
entry:
  %1 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i64 0, i32 5
  %2 = bitcast {}** %1 to i8**
  %RuntimeInterface9 = load i8*, i8** %2, align 1
  %3 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i64 0, i32 6
  %4 = bitcast {}** %3 to i8**
  %Block2KernelMapper10 = load i8*, i8** %4, align 1
  %pSB_LocalId4 = alloca %struct.ndrange_t.6, align 8
  %5 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i64 0, i32 5
  %6 = bitcast {}** %5 to i8**
  %RuntimeInterface78 = load i8*, i8** %6, align 1
  %7 = addrspacecast i8* %RuntimeInterface78 to i8 addrspace(4)*
  %call3.i = tail call %opencl.queue_t.5* @ocl20_get_default_queue(i8 addrspace(4)* %7) #4
  %8 = addrspacecast i8* %RuntimeInterface9 to i8 addrspace(4)*
  %9 = addrspacecast i8* %Block2KernelMapper10 to i8 addrspace(4)*
  %10 = bitcast {}* %RuntimeHandle to i8*
  %11 = addrspacecast i8* %10 to i8 addrspace(4)*
  %ndrange.ascast.i = addrspacecast %struct.ndrange_t.6* %pSB_LocalId4 to %struct.ndrange_t.6 addrspace(4)*
  %12 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 40
  %block.invoke.i.i = getelementptr inbounds i8, i8* %pSpecialBuf, i64 48
  %13 = bitcast i8* %block.invoke.i.i to i8 addrspace(4)**
  store i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8 addrspace(4)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*, i64*, [4 x i64], i8*, {}*)* @__block_fn_block_invoke to i8*) to i8 addrspace(4)*), i8 addrspace(4)** %13, align 8
  %14 = addrspacecast i8* %12 to i8 addrspace(4)*

; CHECK-LABEL: @__block_fn_block_invoke_kernel(
; CHECK: call i32 @__ocl20_enqueue_kernel_basic
; CHECK-SAME: i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8*, i64*, {}*)* @__block_fn_block_invoke_kernel to i8*) to i8 addrspace(4)*)

  %call4.i = call i32 @__ocl20_enqueue_kernel_basic(%opencl.queue_t.5* %call3.i, i32 1, %struct.ndrange_t.6 addrspace(4)* %ndrange.ascast.i, i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8 addrspace(4)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*, i64*, [4 x i64], i8*, {}*)* @__block_fn_block_invoke_kernel to i8*) to i8 addrspace(4)*), i8 addrspace(4)* %14, i8 addrspace(4)* %8, i8 addrspace(4)* %9, i8 addrspace(4)* %11) #5

  ret void
}

declare %opencl.queue_t.5* @ocl20_get_default_queue(i8 addrspace(4)*) local_unnamed_addr #2

declare i32 @__ocl20_enqueue_kernel_basic(%opencl.queue_t.5*, i32, %struct.ndrange_t.6 addrspace(4)*, i8 addrspace(4)*, i8 addrspace(4)*, i8 addrspace(4)*, i8 addrspace(4)*, i8 addrspace(4)*) local_unnamed_addr #3

attributes #0 = { convergent nounwind "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #1 = { nounwind }
attributes #2 = { convergent nofree nounwind readnone willreturn mustprogress "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #3 = { convergent "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #4 = { convergent nounwind readnone willreturn }
attributes #5 = { convergent nounwind }

!sycl.kernels = !{!0}

!0 = !{void (i8 addrspace(4)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*, i64*, [4 x i64], i8*, {}*)* @__block_fn_block_invoke_kernel}
!1 = !{i1 false}
!2 = !{i32 32}
!3 = !{i32 0}
!4 = !{i32 112}
!5 = !{i32 29}
!6 = !{i32 128}
!7 = !{i32 1}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} addrspacecast
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} bitcast
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} load
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} load
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} load
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} load
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} load
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} load
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} load
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} load
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} load
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} mul
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} add
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} mul
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} add
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} mul
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} add
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} insertvalue
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} insertvalue
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} insertvalue
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} mul
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} mul
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} mul
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}} alloca
; DEBUGIFY-NOT: WARNING
