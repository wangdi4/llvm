; RUN: oclopt -add-implicit-args -resolve-wi-call -S %s -o - | FileCheck %s
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: nounwind
define void @A(float addrspace(1)* nocapture %a, float addrspace(1)* nocapture %b, float addrspace(1)* nocapture %c, i32 %iNumElements) #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_type_qual !4 !kernel_arg_name !5 !barrier_buffer_size !12 !kernel_execution_length !13 !kernel_has_barrier !14 !no_barrier_path !15 !vectorized_width !17 {
; CHECK: define void @A
; CHECK: %temp_arg_buf = alloca [4 x i8]
; CHECK: [[GEP0:%[a-zA-Z0-9_]+]] = getelementptr inbounds [4 x i8], [4 x i8]* %temp_arg_buf, i32 0, i32 0
; CHECK: [[BC0:%[a-zA-Z0-9_]+]] = bitcast i8* [[GEP0]] to i32*
; CHECK: store i32 %iNumElements, i32* [[BC0]], align 1
; CHECK: [[GEP1:%[a-zA-Z0-9_]+]] = getelementptr inbounds [4 x i8], [4 x i8]* %temp_arg_buf, i32 0, i32 0
; CHECK: %translated_opencl_printf_call = call i32 @opencl_printf(i8 addrspace(2)* addrspacecast (i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0) to i8 addrspace(2)*), i8* [[GEP1]], {}* %RuntimeInterface, {}* %RuntimeHandle)
; CHECK: ret void
  %call1 = tail call i32 (i8 addrspace(2)*, ...) @printf(i8 addrspace(2)* addrspacecast (i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0) to i8 addrspace(2)*), i32 %iNumElements) #0
  ret void
}

declare i32 @printf(i8 addrspace(2)*, ...)

attributes #0 = { nounwind }

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}
!llvm.functions_info = !{}

!0 = !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32)* @A}
!1 = !{i32 1, i32 1, i32 1, i32 0}
!2 = !{!"none", !"none", !"none", !"none"}
!3 = !{!"float*", !"float*", !"float*", !"int"}
!4 = !{!"const", !"const", !"", !""}
!5 = !{!"a", !"b", !"c", !"iNumElements"}
!6 = !{i32 1, i32 0}
!7 = !{i32 0, i32 0}
!8 = !{}
!12 = !{i32 0}
!13 = !{i32 2}
!14 = !{i1 false}
!15 = !{i1 true}
!17 = !{i32 4}
