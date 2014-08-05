; RUN: oclopt -add-implicit-args -resolve-wi-call -S %s -o - | FileCheck %s
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define void @A(float addrspace(1)* nocapture %a, float addrspace(1)* nocapture %b, float addrspace(1)* nocapture %c, i32 %iNumElements) nounwind {
; CHECK: define void @A
; CHECK: %temp_arg_buf = alloca [4 x i8]
; CHECK: [[GEP0:%[a-zA-Z0-9_]+]] = getelementptr inbounds [4 x i8]* %temp_arg_buf, i32 0, i32 0
; CHECK: [[BC0:%[a-zA-Z0-9_]+]] = bitcast i8* [[GEP0]] to i32*
; CHECK: store i32 %iNumElements, i32* [[BC0]], align 1
; CHECK: [[GEP1:%[a-zA-Z0-9_]+]] = getelementptr inbounds [4 x i8]* %temp_arg_buf, i32 0, i32 0
; CHECK: %translated_opencl_printf_call = call i32 @opencl_printf(i8 addrspace(2)* addrspacecast ([4 x i8]* @.str to i8 addrspace(2)*), i8* [[GEP1]], {}* %RuntimeInterface, {}* %RuntimeHandle)
; CHECK: ret void
  %call1 = tail call i32 (i8 addrspace(2)*, ...)* @printf(i8 addrspace(2)* addrspacecast ([4 x i8]* @.str to i8 addrspace(2)*), i32 %iNumElements) nounwind
  ret void
}

declare i32 @printf(i8 addrspace(2)*, ...)

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}
!opencl.kernel_info = !{!9}
!llvm.functions_info = !{}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32)* @A, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5}
!1 = metadata !{metadata !"kernel_arg_addr_space", i32 1, i32 1, i32 1, i32 0}
!2 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none", metadata !"none", metadata !"none", metadata !"none"}
!3 = metadata !{metadata !"kernel_arg_type", metadata !"float*", metadata !"float*", metadata !"float*", metadata !"int"}
!4 = metadata !{metadata !"kernel_arg_type_qual", metadata !"const", metadata !"const", metadata !"", metadata !""}
!5 = metadata !{metadata !"kernel_arg_name", metadata !"a", metadata !"b", metadata !"c", metadata !"iNumElements"}
!6 = metadata !{i32 1, i32 0}
!7 = metadata !{i32 0, i32 0}
!8 = metadata !{}
!9 = metadata !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32)* @A, metadata !10}
!10 = metadata !{metadata !11, metadata !12, metadata !13, metadata !14, metadata !15, metadata !16, metadata !17, metadata !18, metadata !19}
!11 = metadata !{metadata !"local_buffer_size", null}
!12 = metadata !{metadata !"barrier_buffer_size", i32 0}
!13 = metadata !{metadata !"kernel_execution_length", i32 2}
!14 = metadata !{metadata !"kernel_has_barrier", i1 false}
!15 = metadata !{metadata !"no_barrier_path", i1 true}
!16 = metadata !{metadata !"vectorized_kernel", null}
!17 = metadata !{metadata !"vectorized_width", i32 4}
!18 = metadata !{metadata !"kernel_wrapper", null}
!19 = metadata !{metadata !"scalarized_kernel", null}
