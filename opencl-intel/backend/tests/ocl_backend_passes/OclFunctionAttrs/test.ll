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
define void @KernelFuncCallee1(i32 addrspace(1)* %G, i32 %val, i32 addrspace(3)* %L1, i32 addrspace(3)* %L2) nounwind !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_type_qual !4 !kernel_arg_name !5 {
; CHECK: define void @KernelFuncCallee1(i32 addrspace(1)* noalias %G, i32 %val, i32 addrspace(3)* %L1, i32 addrspace(3)* %L2)
  ret void
}

; kernel function is called by KernelFunc and has one local-mem argument, so we can set NoAlias
define void @KernelFuncCallee2(i32 addrspace(1)* %G, i32 %val, i32 addrspace(3)* %L1) nounwind !kernel_arg_addr_space !7 !kernel_arg_access_qual !8 !kernel_arg_type !9 !kernel_arg_type_qual !10 !kernel_arg_name !11 {
; CHECK: define void @KernelFuncCallee2(i32 addrspace(1)* noalias %G, i32 %val, i32 addrspace(3)* noalias %L1)
  ret void
}

; Kernel function has local-mem arguments and is not called by other functions, so we can set NoAlias
define void @KernelFunc(i32 addrspace(1)* %G, i32 %val, i32 addrspace(3)* %L1, i32 addrspace(3)* %L2) nounwind !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_type_qual !4 !kernel_arg_name !5 {
; CHECK: define void @KernelFunc(i32 addrspace(1)* noalias %G, i32 %val, i32 addrspace(3)* noalias %L1, i32 addrspace(3)* noalias %L2)
  call void @KernelFuncCallee1(i32 addrspace(1)* %G, i32 %val, i32 addrspace(3)* %L1, i32 addrspace(3)* %L1)
  call void @KernelFuncCallee2(i32 addrspace(1)* %G, i32 %val, i32 addrspace(3)* %L2)
  ret void
}

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!13}
!opencl.ocl.version = !{!14}
!opencl.used.extensions = !{!15}
!opencl.used.optional.core.features = !{!15}
!opencl.compiler.options = !{!15}

!0 = !{void (i32 addrspace(1)*, i32, i32 addrspace(3)*, i32 addrspace(3)*)* @KernelFuncCallee1, void (i32 addrspace(1)*, i32, i32 addrspace(3)*)* @KernelFuncCallee2, void (i32 addrspace(1)*, i32, i32 addrspace(3)*, i32 addrspace(3)*)* @KernelFunc}
!1 = !{i32 1, i32 2, i32 0, i32 3, i32 3}
!2 = !{!"none", !"none", !"none", !"none", !"none"}
!3 = !{!"int*", !"int*", !"int", !"int*", !"int*"}
!4 = !{!"", !"const", !"", !"", !""}
!5 = !{!"G", !"C", !"val", !"L1", !"L2"}
!7 = !{i32 1, i32 2, i32 0, i32 3}
!8 = !{!"none", !"none", !"none", !"none"}
!9 = !{!"int*", !"int*", !"int", !"int*"}
!10 = !{!"", !"const", !"", !""}
!11 = !{!"G", !"C", !"val", !"L1"}
!13 = !{i32 1, i32 0}
!14 = !{i32 0, i32 0}
!15 = !{}
