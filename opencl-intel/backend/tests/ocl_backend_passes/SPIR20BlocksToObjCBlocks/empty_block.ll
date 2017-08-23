;;kernel void block_empty(__global int* res)
;;{
;;  void (^kernelBlock)(void) = ^(){};
;;  queue_t def_q = get_default_queue();
;;  ndrange_t ndrange = ndrange_1D(1);
;;  enqueue_kernel(def_q, CLK_ENQUEUE_FLAGS_WAIT_KERNEL, ndrange, kernelBlock);
;;}

; Check that for empty block, invoke function is fixed by SPIR20BlocksToObjCBlocks::fixBlockInvoke()
; That allows to correctly compute block literal size by CloneBlockInvokeFuncToKernel::computeBlockLiteralSize()

; RUN: opt -S -spir20-to-objc-blocks --verify < %s | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%0 = type <{}>
%struct.ndrange_t.2 = type { i32, [3 x i64], [3 x i64], [3 x i64] }
%opencl.queue_t.1 = type opaque
%opencl.block = type opaque
%opencl.clk_event_t.3 = type opaque

; Function Attrs: nounwind
define void @block_empty(i32 addrspace(1)* %res) #0 {
entry:
  %ndrange = alloca %struct.ndrange_t.2, align 8
  %call = call %opencl.queue_t.1* @_Z17get_default_queuev() #0
  call void @_Z10ndrange_1Dmmm(%struct.ndrange_t.2* %ndrange, i64 0, i64 1, i64 0)
  %block = call %opencl.block* @spir_block_bind(i8* bitcast (void (i8*)* @__block_empty_block_invoke to i8*), i32 0, i32 0, i8* null)
  %call1 = call i32 @_Z14enqueue_kernel9ocl_queuei9ndrange_tjPU3AS4K12ocl_clkeventPS1_U13block_pointerFvvE(%opencl.queue_t.1* %call, i32 1, %struct.ndrange_t.2* %ndrange, i32 0, %opencl.clk_event_t.3* addrspace(4)* null, %opencl.clk_event_t.3* addrspace(4)* null, %opencl.block* %block)
  ret void
}

; Function Attrs: nounwind
declare %opencl.queue_t.1* @_Z17get_default_queuev() #0

; Function Attrs: nounwind
define internal void @__block_empty_block_invoke(i8* %.block_descriptor) #0 {
entry:
  %block = bitcast i8* %.block_descriptor to %0*
; CHECK: %spir.to.ojbc.cast = bitcast i8* %.block_descriptor to <{ i8*, i32, i32, i8*, %struct.__block_descriptor* }>*
; CHECK-NOT: %block = bitcast i8* %.block_descriptor to %0*
  ret void
}

declare void @_Z10ndrange_1Dmmm(%struct.ndrange_t.2*, i64, i64, i64)

declare %opencl.block* @spir_block_bind(i8*, i32, i32, i8*)

declare i32 @_Z14enqueue_kernel9ocl_queuei9ndrange_tjPU3AS4K12ocl_clkeventPS1_U13block_pointerFvvE(%opencl.queue_t.1*, i32, %struct.ndrange_t.2*, i32, %opencl.clk_event_t.3* addrspace(4)*, %opencl.clk_event_t.3* addrspace(4)*, %opencl.block*)

attributes #0 = { nounwind }

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!spirv.Source = !{!6}
!opencl.spir.version = !{!7}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!spirv.Generator = !{!9}
!opencl.compiler.options = !{!10}

!0 = !{void (i32 addrspace(1)*)* @block_empty, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1}
!2 = !{!"kernel_arg_access_qual", !"none"}
!3 = !{!"kernel_arg_type", !"int*"}
!4 = !{!"kernel_arg_type_qual", !""}
!5 = !{!"kernel_arg_base_type", !"int*"}
!6 = !{i32 3, i32 200000}
!7 = !{i32 2, i32 0}
!8 = !{}
!9 = !{i16 6, i16 14}
!10 = !{!"-cl-std=CL2.0"}

