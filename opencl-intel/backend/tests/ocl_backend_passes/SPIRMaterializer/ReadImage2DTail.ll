; RUN: opt -spir-materializer -S %s -o - | FileCheck %s

; CHECK: %opencl.image2d_ro_t.134
; CHECK-NOT: %opencl.image2d_t.13ro_4
; CHECK-NOT: call <4 x i32> @_Z11read_imagei14ocl_image2d11ocl_samplerDv2_i

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%opencl.image2d_t.134 = type opaque

; Function Attrs: nounwind
define spir_kernel void @img_read_kernel(%opencl.image2d_t.134 addrspace(1)* %img, i32 %sampler, i32 addrspace(1)* %outbuf) #0 {
entry:
  %call.old = call spir_func <4 x i32> @_Z11read_imagei11ocl_image2d11ocl_samplerDv2_i(%opencl.image2d_t.134 addrspace(1)* %img, i32 %sampler, <2 x i32> zeroinitializer)
  %0 = extractelement <4 x i32> %call.old, i32 0
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %outbuf, i64 7
  store i32 %0, i32 addrspace(1)* %arrayidx, align 4
  %1 = extractelement <4 x i32> %call.old, i32 1
  %arrayidx1 = getelementptr inbounds i32, i32 addrspace(1)* %outbuf, i64 8
  store i32 %1, i32 addrspace(1)* %arrayidx1, align 4
  %2 = extractelement <4 x i32> %call.old, i32 2
  %arrayidx2 = getelementptr inbounds i32, i32 addrspace(1)* %outbuf, i64 9
  store i32 %2, i32 addrspace(1)* %arrayidx2, align 4
  %3 = extractelement <4 x i32> %call.old, i32 3
  %arrayidx3 = getelementptr inbounds i32, i32 addrspace(1)* %outbuf, i64 10
  store i32 %3, i32 addrspace(1)* %arrayidx3, align 4
  ret void
}

; Function Attrs: nounwind
declare spir_func <4 x i32> @_Z11read_imagei11ocl_image2d11ocl_samplerDv2_i(%opencl.image2d_t.134 addrspace(1)*, i32, <2 x i32>) #0

; Function Attrs: nounwind
define spir_kernel void @img_write_kernel(%opencl.image2d_t.134 addrspace(1)* %img, <4 x i32> %color) #0 {
entry:
  call spir_func void @_Z12write_imagei11ocl_image2dDv2_iDv4_i(%opencl.image2d_t.134 addrspace(1)* %img, <2 x i32> zeroinitializer, <4 x i32> %color)
  ret void
}

; Function Attrs: nounwind
declare spir_func void @_Z12write_imagei11ocl_image2dDv2_iDv4_i(%opencl.image2d_t.134 addrspace(1)*, <2 x i32>, <4 x i32>) #0

attributes #0 = { nounwind }

!opencl.kernels = !{!0, !6}
!opencl.enable.FP_CONTRACT = !{}
!spirv.Source = !{!12}
!opencl.spir.version = !{!13}
!opencl.ocl.version = !{!13}
!opencl.used.extensions = !{!14}
!opencl.used.optional.core.features = !{!15}
!spirv.Generator = !{!16}
!opencl.compiler.options = !{!14}

!0 = !{void (%opencl.image2d_t.134 addrspace(1)*, i32, i32 addrspace(1)*)* @img_read_kernel, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 0, i32 1}
!2 = !{!"kernel_arg_access_qual", !"read_only", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"image2d_t", !"sampler_t", !"int*"}
!4 = !{!"kernel_arg_type_qual", !"", !"", !""}
!5 = !{!"kernel_arg_base_type", !"image2d_t", !"sampler_t", !"int*"}
!6 = !{void (%opencl.image2d_t.134 addrspace(1)*, <4 x i32>)* @img_write_kernel, !7, !8, !9, !10, !11}
!7 = !{!"kernel_arg_addr_space", i32 1, i32 0}
!8 = !{!"kernel_arg_access_qual", !"write_only", !"none"}
!9 = !{!"kernel_arg_type", !"image2d_t", !"int4"}
!10 = !{!"kernel_arg_type_qual", !"", !""}
!11 = !{!"kernel_arg_base_type", !"image2d_t", !"int4"}
!12 = !{i32 3, i32 102000}
!13 = !{i32 1, i32 2}
!14 = !{}
!15 = !{!"cl_images"}
!16 = !{i16 6, i16 14}
