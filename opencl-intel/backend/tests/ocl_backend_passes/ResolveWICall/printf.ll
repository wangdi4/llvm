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

!0 = !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32)* @A, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 1, i32 1, i32 0}
!2 = !{!"kernel_arg_access_qual", !"none", !"none", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"float*", !"float*", !"float*", !"int"}
!4 = !{!"kernel_arg_type_qual", !"const", !"const", !"", !""}
!5 = !{!"kernel_arg_name", !"a", !"b", !"c", !"iNumElements"}
!6 = !{i32 1, i32 0}
!7 = !{i32 0, i32 0}
!8 = !{}
!9 = !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32)* @A, !10}
!10 = !{!11, !12, !13, !14, !15, !16, !17, !18, !19}
!11 = !{!"local_buffer_size", null}
!12 = !{!"barrier_buffer_size", i32 0}
!13 = !{!"kernel_execution_length", i32 2}
!14 = !{!"kernel_has_barrier", i1 false}
!15 = !{!"no_barrier_path", i1 true}
!16 = !{!"vectorized_kernel", null}
!17 = !{!"vectorized_width", i32 4}
!18 = !{!"kernel_wrapper", null}
!19 = !{!"scalarized_kernel", null}
