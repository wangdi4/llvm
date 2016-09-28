; RUN: opt -spir-materializer -S %s -o - | FileCheck %s

; CHECK: %opencl.image2d_ro_t
; CHECK: @_Z15get_image_width14ocl_image2d_ro
; CHECK: @_Z12read_imageui14ocl_image2d_roDv2_i
; CHECK: @_Z12read_imageui14ocl_image2d_ro11ocl_samplerDv2_i

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024"
target triple = "spir-unknown-unknown"

%opencl.image2d_t = type opaque

; Function Attrs: nounwind
define spir_kernel void @sample_kernel(%opencl.image2d_t addrspace(1)* %input, i32 %sampler, i32 addrspace(1)* nocapture %results) #0 {
  %1 = tail call spir_func i32 @_Z13get_global_idj(i32 0) #1
  %2 = tail call spir_func i32 @_Z13get_global_idj(i32 1) #1
  %3 = tail call spir_func i32 @_Z15get_image_width11ocl_image2d(%opencl.image2d_t addrspace(1)* %input) #1
  %4 = mul nsw i32 %3, %2
  %5 = add nsw i32 %4, %1
  %6 = insertelement <2 x i32> undef, i32 %1, i32 0
  %7 = insertelement <2 x i32> %6, i32 %2, i32 1
  %8 = tail call spir_func <4 x i32> @_Z12read_imageui11ocl_image2dDv2_i(%opencl.image2d_t addrspace(1)* %input, <2 x i32> %7) #1
  %9 = tail call spir_func <4 x i32> @_Z12read_imageui11ocl_image2d11ocl_samplerDv2_i(%opencl.image2d_t addrspace(1)* %input, i32 %sampler, <2 x i32> %7) #1
  %10 = icmp ne <4 x i32> %8, %9
  %11 = sext <4 x i1> %10 to <4 x i32>
  %12 = extractelement <4 x i32> %11, i32 0
  %13 = extractelement <4 x i32> %11, i32 1
  %14 = or i32 %12, %13
  %15 = extractelement <4 x i32> %11, i32 2
  %16 = or i32 %14, %15
  %17 = extractelement <4 x i32> %11, i32 3
  %18 = or i32 %16, %17
  %19 = getelementptr inbounds i32, i32 addrspace(1)* %results, i32 %5
  %not. = icmp ne i32 %18, 0
  %. = sext i1 %not. to i32
  store i32 %., i32 addrspace(1)* %19, align 4
  ret void
}

; Function Attrs: nounwind readnone
declare spir_func i32 @_Z13get_global_idj(i32) #1

; Function Attrs: nounwind readnone
declare spir_func i32 @_Z15get_image_width11ocl_image2d(%opencl.image2d_t addrspace(1)*) #1

; Function Attrs: nounwind readnone
declare spir_func <4 x i32> @_Z12read_imageui11ocl_image2dDv2_i(%opencl.image2d_t addrspace(1)*, <2 x i32>) #1

; Function Attrs: nounwind readnone
declare spir_func <4 x i32> @_Z12read_imageui11ocl_image2d11ocl_samplerDv2_i(%opencl.image2d_t addrspace(1)*, i32, <2 x i32>) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!7}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!9}
!opencl.compiler.options = !{!10}

!0 = !{void (%opencl.image2d_t addrspace(1)*, i32, i32 addrspace(1)*)* @sample_kernel, !1, !2, !3, !4, !5, !6}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 0, i32 1}
!2 = !{!"kernel_arg_access_qual", !"read_only", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"image2d_uint", !"sampler_t", !"int*"}
!4 = !{!"kernel_arg_type_qual", !"", !"", !""}
!5 = !{!"kernel_arg_base_type", !"image2d_t", !"sampler_t", !"int*"}
!6 = !{!"kernel_arg_name", !"input", !"sampler", !"results"}
!7 = !{i32 1, i32 2}
!8 = !{}
!9 = !{!"cl_images"}
!10 = !{!"-cl-kernel-arg-info"}
