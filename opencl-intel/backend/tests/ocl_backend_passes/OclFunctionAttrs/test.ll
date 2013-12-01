; RUN: opt -ocl-functionattrs -S < %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Non-kernel function has more than one local-mem argument with NoAlias, so we cannot set NoAlias to any local-mem args
; %C1 is the only constant/global-mem argument without NoAlias, so we can set it to NoAlias
define void @NonKernelFunc1(i32 addrspace(1)* noalias %G, i32 addrspace(2)* %C1, i32 addrspace(2)* noalias %C2, i32 addrspace(2)* noalias %C3, i32 %val, i32 addrspace(3)* %L1, i32 addrspace(3)* %L2) nounwind {
; CHECK: define void @NonKernelFunc1(i32 addrspace(1)* noalias %G, i32 addrspace(2)* noalias %C1, i32 addrspace(2)* noalias %C2, i32 addrspace(2)* noalias %C3, i32 %val, i32 addrspace(3)* %L1, i32 addrspace(3)* %L2)
  ret void
}

; Non-kernel function has one of each addrspace argument. Global and constant may alias, so we can set only the local to NoAlias
define void @NonKernelFunc2(i32 addrspace(1)* %G, i32 addrspace(2)* %C, i32 %val, i32 addrspace(3)* %L) nounwind {
; CHECK: define void @NonKernelFunc2(i32 addrspace(1)* %G, i32 addrspace(2)* %C, i32 %val, i32 addrspace(3)* noalias %L)
  ret void
}

; function has a generic argument without NoAlias, so we cannot do anything
define void @NonKernelFunc3(i32 addrspace(4)* %X, i32 addrspace(1)* %G, i32 %val, i32 addrspace(3)* %L) nounwind {
; CHECK: define void @NonKernelFunc3(i32 addrspace(4)* %X, i32 addrspace(1)* %G, i32 %val, i32 addrspace(3)* %L)
  ret void
}

; function has a generic argument with NoAlias, so we cannot do everything
define void @NonKernelFunc4(i32 addrspace(4)* noalias %X, i32 addrspace(1)* %G, i32 %val, i32 addrspace(3)* %L) nounwind {
; CHECK: define void @NonKernelFunc4(i32 addrspace(4)* noalias %X, i32 addrspace(1)* noalias %G, i32 %val, i32 addrspace(3)* noalias %L)
  ret void
}

; kernel function is called by KernelFunc and more than one local-mem argument, so we cannot set NoAlias
define void @KernelFuncCallee1(i32 addrspace(1)* %G, i32 %val, i32 addrspace(3)* %L1, i32 addrspace(3)* %L2) nounwind {
; CHECK: define void @KernelFuncCallee1(i32 addrspace(1)* noalias %G, i32 %val, i32 addrspace(3)* %L1, i32 addrspace(3)* %L2)
  ret void
}

; kernel function is called by KernelFunc and has one local-mem argument, so we can set NoAlias
define void @KernelFuncCallee2(i32 addrspace(1)* %G, i32 %val, i32 addrspace(3)* %L1) nounwind {
; CHECK: define void @KernelFuncCallee2(i32 addrspace(1)* noalias %G, i32 %val, i32 addrspace(3)* noalias %L1)
  ret void
}

; Kernel function has local-mem arguments and is not called by other functions, so we can set NoAlias
define void @KernelFunc(i32 addrspace(1)* %G, i32 %val, i32 addrspace(3)* %L1, i32 addrspace(3)* %L2) nounwind {
; CHECK: define void @KernelFunc(i32 addrspace(1)* noalias %G, i32 %val, i32 addrspace(3)* noalias %L1, i32 addrspace(3)* noalias %L2)
  call void @KernelFuncCallee1(i32 addrspace(1)* %G, i32 %val, i32 addrspace(3)* %L1, i32 addrspace(3)* %L1)
  call void @KernelFuncCallee2(i32 addrspace(1)* %G, i32 %val, i32 addrspace(3)* %L2)
  ret void
}

!opencl.kernels = !{!0, !6, !12}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!13}
!opencl.ocl.version = !{!14}
!opencl.used.extensions = !{!15}
!opencl.used.optional.core.features = !{!15}
!opencl.compiler.options = !{!15}

!0 = metadata !{void (i32 addrspace(1)*, i32, i32 addrspace(3)*, i32 addrspace(3)*)* @KernelFuncCallee1, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5}
!1 = metadata !{metadata !"kernel_arg_addr_space", i32 1, i32 2, i32 0, i32 3, i32 3}
!2 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none", metadata !"none", metadata !"none", metadata !"none", metadata !"none"}
!3 = metadata !{metadata !"kernel_arg_type", metadata !"int*", metadata !"int*", metadata !"int", metadata !"int*", metadata !"int*"}
!4 = metadata !{metadata !"kernel_arg_type_qual", metadata !"", metadata !"const", metadata !"", metadata !"", metadata !""}
!5 = metadata !{metadata !"kernel_arg_name", metadata !"G", metadata !"C", metadata !"val", metadata !"L1", metadata !"L2"}
!6 = metadata !{void (i32 addrspace(1)*, i32, i32 addrspace(3)*)* @KernelFuncCallee2, metadata !7, metadata !8, metadata !9, metadata !10, metadata !11}
!7 = metadata !{metadata !"kernel_arg_addr_space", i32 1, i32 2, i32 0, i32 3}
!8 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none", metadata !"none", metadata !"none", metadata !"none"}
!9 = metadata !{metadata !"kernel_arg_type", metadata !"int*", metadata !"int*", metadata !"int", metadata !"int*"}
!10 = metadata !{metadata !"kernel_arg_type_qual", metadata !"", metadata !"const", metadata !"", metadata !""}
!11 = metadata !{metadata !"kernel_arg_name", metadata !"G", metadata !"C", metadata !"val", metadata !"L1"}
!12 = metadata !{void (i32 addrspace(1)*, i32, i32 addrspace(3)*, i32 addrspace(3)*)* @KernelFunc, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5}
!13 = metadata !{i32 1, i32 0}
!14 = metadata !{i32 0, i32 0}
!15 = metadata !{}
