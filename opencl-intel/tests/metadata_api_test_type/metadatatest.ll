; ModuleID = 'metadatatest.cl'
; The IR below was generated using the following OpenCL C source code
;
; __attribute__((vec_type_hint(float)))
; __attribute__((work_group_size_hint(8,16,32)))
; __attribute__((reqd_work_group_size(1,2,4)))
; __attribute__((intel_reqd_sub_group_size(1)))
; __kernel void metatest_kernel(float argFloat, __global int * argIntBuffer, __read_only image2d_t argImg) {
;     return;
; }
;
; The IR below was generated with the following command
; $ ./clang -cc1 -emit-llvm -O0 -x cl -triple spir64-unknown-unknown -cl-kernel-arg-info metadatatest.cl -o backend/metadata_api/tests/metadatatest.ll
;
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

%opencl.image2d_ro_t = type opaque

; Function Attrs: convergent noinline norecurse nounwind optnone
define spir_kernel void @metatest_kernel(float %argFloat, i32 addrspace(1)* %argIntBuffer, %opencl.image2d_ro_t addrspace(1)* %argImg) #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_name !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !8 !kernel_arg_buffer_location !8 !vec_type_hint !12 !work_group_size_hint !13 !reqd_work_group_size !14 !intel_reqd_sub_group_size !15 {
entry:
  %argFloat.addr = alloca float, align 4
  %argIntBuffer.addr = alloca i32 addrspace(1)*, align 8
  %argImg.addr = alloca %opencl.image2d_ro_t addrspace(1)*, align 8
  store float %argFloat, float* %argFloat.addr, align 4
  store i32 addrspace(1)* %argIntBuffer, i32 addrspace(1)** %argIntBuffer.addr, align 8
  store %opencl.image2d_ro_t addrspace(1)* %argImg, %opencl.image2d_ro_t addrspace(1)** %argImg.addr, align 8
  ret void
}

; Function Attrs: noinline nounwind
define spir_kernel void @metatest_plain_func() #0 {
entry:
  ret void
}

attributes #0 = { convergent noinline norecurse nounwind optnone "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }

!sycl.kernels = !{!16}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!4}

!0 = !{i32 1, i32 0}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"cl_images"}
!4 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!5 = !{i32 0, i32 1, i32 1}
!6 = !{!"none", !"none", !"read_only"}
!7 = !{!"float", !"int*", !"image2d_t"}
!8 = !{!"", !"", !""}
!9 = !{!"argFloat", !"argIntBuffer", !"argImg"}
!10 = !{i1 false, i1 false, i1 false}
!11 = !{i32 0, i32 0, i32 0}
!12 = !{float undef, i32 0}
!13 = !{i32 8, i32 16, i32 32}
!14 = !{i32 1, i32 2, i32 4}
!15 = !{i32 1}
!16 = !{void (float, i32 addrspace(1)*, %opencl.image2d_ro_t addrspace(1)*)* @metatest_kernel}
