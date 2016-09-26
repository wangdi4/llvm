;;kernel void block_empty(__global int* res)
;;{
;;  void (^kernelBlock)(void) = ^(){};
;;  kernelBlock();
;;}

;; __block_empty_block_invoke function should not be processed by this pass. Otherwise it may crash.
; RUN: opt -cloneblockinvokefunctokernel <  %s -S

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: nounwind
define void @block_empty(i32 addrspace(1)* %res) #0 {
entry:
  call void @__block_empty_block_invoke() #0
  ret void
}

; Function Attrs: nounwind
define internal void @__block_empty_block_invoke() #0 {
entry:
  ret void
}

attributes #0 = { nounwind }

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!spirv.Source = !{!6}
!opencl.spir.version = !{!7}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!spirv.Generator = !{!9}
!opencl.compiler.options = !{!10}

!0 = !{void (i32 addrspace(1)*)* @block_empty, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1}
!2 = !{!"kernel_arg_access_qual", !"none"}
!3 = !{!"kernel_arg_type", !"int*"}
!4 = !{!"kernel_arg_type_qual", !""}
!5 = !{!"kernel_arg_base_type", !"int*"}
!6 = !{i32 3, i32 200000}
!7 = !{i32 2, i32 0}
!8 = !{}
!9 = !{i16 6, i16 14}
!10 = !{!"-cl-std=CL2.0"}

