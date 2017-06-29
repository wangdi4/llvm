; You can find CL source file in the same directory where this file is located
;
;***************************************************************************************
; The test checks %opencl.block opaque type to function pointer cast for block argument
; for device execution builtins.
;***************************************************************************************
;
; RUN: opt -S -block-to-func-ptr -runtimelib=%S/extended_execution_functions.bc --verify < %s | FileCheck %s

; CHECK: declare {{.*}} @_Z14enqueue_kernel{{.*}}, void ()*)
; CHECK: [[TOFUNCPTR:.*]] = bitcast %opencl.block* {{.*}} to  void ()*
; CHECK: call {{.*}} @_Z14enqueue_kernel{{.*}}, void ()*[[TOFUNCPTR]])
; CHECK-NOT: call {{.*}}@_Z14enqueue_kernel{{.*}}, %opencl.block* {{.*}})

; ModuleID = 'backend/tests/ocl_backend_passes/BlockToFuncPtr/block_to_func_ptr.0.ll'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024"
target triple = "spir64-unknown-unknown"

%0 = type <{ float addrspace(1)* }>
%struct.__block_descriptor = type { i64, i64 }
%opencl.block = type opaque
%struct.ndrange_t = type { i32, [3 x i64], [3 x i64], [3 x i64] }
%opencl.queue_t = type opaque
%opencl.clk_event_t = type opaque

@__block_descriptor.objc = internal constant %struct.__block_descriptor { i64 0, i64 40 }

; Function Attrs: nounwind
define spir_kernel void @device_kernel(float addrspace(1)* %inout) #0 {
entry:
  %0 = load float, float addrspace(1)* %inout, align 4
  %call = call spir_func float @_Z3cosf(float %0) #0
  store float %call, float addrspace(1)* %inout, align 4
  ret void
}

; Function Attrs: nounwind
declare spir_func float @_Z3cosf(float) #0

; Function Attrs: nounwind
define spir_kernel void @host_kernel(float addrspace(1)* %inout) #0 {
entry:
  %objc.block = alloca <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, float addrspace(1)* }>, align 8
  %objc.block.invoke = getelementptr <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, float addrspace(1)* }>, <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, float addrspace(1)* }>* %objc.block, i32 0, i32 3
  store i8* bitcast (void (i8*)* @__host_kernel_block_invoke to i8*), i8** %objc.block.invoke
  %objc.block.descriptor = getelementptr <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, float addrspace(1)* }>, <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, float addrspace(1)* }>* %objc.block, i32 0, i32 4
  store %struct.__block_descriptor* @__block_descriptor.objc, %struct.__block_descriptor** %objc.block.descriptor
  %objc.to.spir.cast = bitcast <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, float addrspace(1)* }>* %objc.block to %opencl.block*
  %captured = alloca %0, align 8
  %agg.tmp = alloca %struct.ndrange_t, align 8
  %call = call spir_func %opencl.queue_t* @_Z17get_default_queuev() #0
  call spir_func void @_Z10ndrange_1Dmmm(%struct.ndrange_t* %agg.tmp, i64 0, i64 1, i64 0)
  %objc.block.captured = getelementptr <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, float addrspace(1)* }>, <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, float addrspace(1)* }>* %objc.block, i32 0, i32 5
  store float addrspace(1)* %inout, float addrspace(1)** %objc.block.captured, align 8
  %0 = bitcast %0* %captured to i8*
  %AddrSpace = addrspacecast %opencl.clk_event_t* addrspace(4)* null to %opencl.clk_event_t* addrspace(1)*
  %AddrSpace1 = addrspacecast %opencl.clk_event_t* addrspace(4)* null to %opencl.clk_event_t* addrspace(1)*
  %1 = call spir_func i32 @_Z14enqueue_kernel9ocl_queuei9ndrange_tjPU3AS1K12ocl_clkeventPU3AS1S2_U13block_pointerFvvE(%opencl.queue_t* %call, i32 1, %struct.ndrange_t* %agg.tmp, i32 0, %opencl.clk_event_t* addrspace(1)* %AddrSpace, %opencl.clk_event_t* addrspace(1)* %AddrSpace1, %opencl.block* %objc.to.spir.cast)
  ret void
}

; Function Attrs: nounwind
declare spir_func %opencl.queue_t* @_Z17get_default_queuev() #0

; Function Attrs: nounwind
define internal spir_func void @__host_kernel_block_invoke(i8* %.block_descriptor) #0 {
entry:
  %spir.to.ojbc.cast = bitcast i8* %.block_descriptor to <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, float addrspace(1)* }>*
  %objc.block.captured = getelementptr <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, float addrspace(1)* }>, <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, float addrspace(1)* }>* %spir.to.ojbc.cast, i32 0, i32 5
  %0 = load float addrspace(1)*, float addrspace(1)** %objc.block.captured, align 8
  call spir_kernel void @device_kernel(float addrspace(1)* %0) #0
  ret void
}

declare spir_func void @_Z10ndrange_1Dmmm(%struct.ndrange_t*, i64, i64, i64)

declare spir_func i32 @_Z14enqueue_kernel9ocl_queuei9ndrange_tjPU3AS4K12ocl_clkeventPU3AS4S2_U13block_pointerFvvE(%opencl.queue_t*, i32, %struct.ndrange_t*, i32, %opencl.clk_event_t* addrspace(4)*, %opencl.clk_event_t* addrspace(4)*, %opencl.block*)

declare spir_func i32 @_Z14enqueue_kernel9ocl_queuei9ndrange_tjPU3AS1K12ocl_clkeventPU3AS1S2_U13block_pointerFvvE(%opencl.queue_t*, i32, %struct.ndrange_t*, i32, %opencl.clk_event_t* addrspace(1)*, %opencl.clk_event_t* addrspace(1)*, %opencl.block*)

attributes #0 = { nounwind }

!opencl.kernels = !{!0, !6}
!opencl.enable.FP_CONTRACT = !{}
!spirv.Source = !{!7}
!opencl.spir.version = !{!8}
!opencl.ocl.version = !{!8}
!opencl.used.extensions = !{!9}
!opencl.used.optional.core.features = !{!9}
!spirv.Generator = !{!10}

!0 = !{void (float addrspace(1)*)* @device_kernel, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1}
!2 = !{!"kernel_arg_access_qual", !"none"}
!3 = !{!"kernel_arg_type", !"float*"}
!4 = !{!"kernel_arg_type_qual", !""}
!5 = !{!"kernel_arg_base_type", !"float*"}
!6 = !{void (float addrspace(1)*)* @host_kernel, !1, !2, !3, !4, !5}
!7 = !{i32 3, i32 200000}
!8 = !{i32 2, i32 0}
!9 = !{}
!10 = !{i16 6, i16 14}
