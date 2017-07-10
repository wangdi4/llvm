; ModuleID = 'metadatatest.cl'
; The IR below was generated with the following command
;
; $ cat metadatatest.cl
; __attribute__((vec_type_hint(float)))
; __attribute__((work_group_size_hint(8,16,32)))
; __attribute__((reqd_work_group_size(1,2,4)))
; __attribute__((intel_reqd_sub_group_size(1)))
; __kernel void metatest_kernel(float argFloat, __global int * argIntBuffer, __read_only image2d_t argImg) {
;     return;
; }
;
; $ ./clang -cc1 -emit-llvm -O0 -x cl -triple spir64-unknown-unknown -cl-kernel-arg-info metadatatest.cl -o backend/metadata_api/tests/metadatatest.ll
;
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

%opencl.image2d_ro_t = type opaque

; Function Attrs: noinline nounwind
define spir_kernel void @metatest_kernel(float %argFloat, i32 addrspace(1)* %argIntBuffer, %opencl.image2d_ro_t addrspace(1)* %argImg) #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !5 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !7 !kernel_arg_name !8 !vec_type_hint !9 !work_group_size_hint !10 !reqd_work_group_size !11 !intel_reqd_sub_group_size !12 {
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

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.kernels = !{!13}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 1, i32 0}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"clang version 4.0.1 (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-clang 3ad7cab41d1677fb8cefaad1196860acbdb74f11) (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-llvm 8456c1092144484dcae55966238f9b9a09d740fe)"}
!4 = !{i32 0, i32 1, i32 1}
!5 = !{!"none", !"none", !"read_only"}
!6 = !{!"float", !"int*", !"image2d_t"}
!7 = !{!"", !"", !""}
!8 = !{!"argFloat", !"argIntBuffer", !"argImg"}
!9 = !{float undef, i32 0}
!10 = !{i32 8, i32 16, i32 32}
!11 = !{i32 1, i32 2, i32 4}
!12 = !{i32 1}
!13 = !{void (float, i32 addrspace(1)*, %opencl.image2d_ro_t addrspace(1)*)* @metatest_kernel}
