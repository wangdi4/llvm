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
; void metatest_plain_func() {}
;
; The IR below was generated with the following command and manually added with !sycl.kernels metadata.
; $ clang -cc1 -triple spir64 -x cl -cl-std=cl1.2 -finclude-default-header -disable-llvm-passes -emit-llvm -o 1.ll metadatatest.cl

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"

; Function Attrs: convergent norecurse nounwind
define dso_local spir_kernel void @metatest_kernel(float noundef %argFloat, ptr addrspace(1) noundef align 4 %argIntBuffer, target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0) %argImg) #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !3 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !5 !kernel_arg_host_accessible !6 !kernel_arg_pipe_depth !7 !kernel_arg_pipe_io !5 !kernel_arg_buffer_location !5 !vec_type_hint !8 !work_group_size_hint !9 !reqd_work_group_size !10 !intel_reqd_sub_group_size !11 {
entry:
  %argFloat.addr = alloca float, align 4
  %argIntBuffer.addr = alloca ptr addrspace(1), align 8
  %argImg.addr = alloca target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0), align 8
  store float %argFloat, ptr %argFloat.addr, align 4, !tbaa !12
  store ptr addrspace(1) %argIntBuffer, ptr %argIntBuffer.addr, align 8, !tbaa !16
  store target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0) %argImg, ptr %argImg.addr, align 8, !tbaa !18
  ret void
}

define dso_local spir_func void @metatest_plain_func() #1 {
entry:
   ret void
 }

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" }
attributes #1 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }

!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!sycl.kernels = !{!1}

!0 = !{i32 1, i32 2}
!1 = !{ptr @metatest_kernel}
!2 = !{i32 0, i32 1, i32 1}
!3 = !{!"none", !"none", !"read_only"}
!4 = !{!"float", !"int*", !"image2d_t"}
!5 = !{!"", !"", !""}
!6 = !{i1 false, i1 false, i1 false}
!7 = !{i32 0, i32 0, i32 0}
!8 = !{float undef, i32 0}
!9 = !{i32 8, i32 16, i32 32}
!10 = !{i32 1, i32 2, i32 4}
!11 = !{i32 1}
!12 = !{!13, !13, i64 0}
!13 = !{!"float", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C/C++ TBAA"}
!16 = !{!17, !17, i64 0}
!17 = !{!"any pointer", !14, i64 0}
!18 = !{!19, !19, i64 0}
!19 = !{!"__read_only image2d_t", !14, i64 0}
