; ModuleID = 'metadatatest.cl'
; The IR below was generated with the following command
; $
; $ cat metadatatest.cl
; $ __attribute__((vec_type_hint(float)))
; $ __kernel void metatest(float argFloat, __global int * argIntBuffer, __read_only image2d_t argImg) {
; $     return;
; $ }
; $
; $ ./clang -cc1 -emit-llvm -O0 -x cl -triple spir-unknown-unknown metadatatest.cl -o backend/metadata_api/tests/metadatatest.ll
; $

target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir-unknown-unknown"

%opencl.image2d_t = type opaque

; Function Attrs: nounwind
define spir_kernel void @metatest(float %argFloat, i32 addrspace(1)* %argIntBuffer, %opencl.image2d_t addrspace(1)* %argImg) #0 {
  %1 = alloca float, align 4
  %2 = alloca i32 addrspace(1)*, align 4
  %3 = alloca %opencl.image2d_t addrspace(1)*, align 4
  store float %argFloat, float* %1, align 4
  store i32 addrspace(1)* %argIntBuffer, i32 addrspace(1)** %2, align 4
  store %opencl.image2d_t addrspace(1)* %argImg, %opencl.image2d_t addrspace(1)** %3, align 4
  ret void
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!7}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!9}
!opencl.compiler.options = !{!8}
!llvm.ident = !{!10}

!0 = !{void (float, i32 addrspace(1)*, %opencl.image2d_t addrspace(1)*)* @metatest, !1, !2, !3, !4, !5, !6}
!1 = !{!"kernel_arg_addr_space", i32 0, i32 1, i32 1}
!2 = !{!"kernel_arg_access_qual", !"none", !"none", !"read_only"}
!3 = !{!"kernel_arg_type", !"float", !"int*", !"image2d_t"}
!4 = !{!"kernel_arg_base_type", !"float", !"int*", !"image2d_t"}
!5 = !{!"kernel_arg_type_qual", !"", !"", !""}
!6 = !{!"vec_type_hint", float undef, i32 1}
!7 = !{i32 1, i32 2}
!8 = !{}
!9 = !{!"cl_images"}
!10 = !{!"clang version 3.6.0 (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-clang 83869a5aa2cc8e6efb5dab84d4f034a88fa5515f) (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-llvm 50546c308a35b18ee2afb43648a5c2b0e414227f)"}
