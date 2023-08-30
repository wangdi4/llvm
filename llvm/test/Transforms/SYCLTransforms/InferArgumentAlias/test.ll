; RUN: opt -passes=sycl-kernel-infer-argument-alias -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-infer-argument-alias -S %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Non-kernel function has no local-mem arguments with Noalias, we cannot set NoAlias to any local-mem args
; %C1 is the only constant/global-mem argument without NoAlias, so we can set it to NoAlias
define void @NonKernelFunc1(ptr addrspace(1) noalias %G, ptr addrspace(2) %C1, ptr addrspace(2) noalias %C2, ptr addrspace(2) noalias %C3, i32 %val, ptr addrspace(3) %L1, ptr addrspace(3) %L2) nounwind !kernel_arg_base_type !16 !arg_type_null_val !17 {
; CHECK: define void @NonKernelFunc1(ptr addrspace(1) noalias %G, ptr addrspace(2) noalias %C1, ptr addrspace(2) noalias %C2, ptr addrspace(2) noalias %C3, i32 %val, ptr addrspace(3) %L1, ptr addrspace(3) %L2)
  ret void
}

; Non-kernel function has one of each addrspace argument. Global and constant may alias, so we can set only the local to NoAlias
define void @NonKernelFunc2(ptr addrspace(1) %G, ptr addrspace(2) %C, i32 %val, ptr addrspace(3) %L) nounwind !kernel_arg_base_type !18 !arg_type_null_val !19 {
; CHECK: define void @NonKernelFunc2(ptr addrspace(1) %G, ptr addrspace(2) %C, i32 %val, ptr addrspace(3) noalias %L)
  ret void
}

; function has a generic argument without NoAlias, so we cannot do anything
define void @NonKernelFunc3(ptr addrspace(4) %X, ptr addrspace(1) %G, i32 %val, ptr addrspace(3) %L) nounwind !kernel_arg_base_type !20 !arg_type_null_val !21 {
; CHECK: define void @NonKernelFunc3(ptr addrspace(4) %X, ptr addrspace(1) %G, i32 %val, ptr addrspace(3) %L)
  ret void
}

; function has a generic argument with NoAlias, so we can do everything
define void @NonKernelFunc4(ptr addrspace(4) noalias %X, ptr addrspace(1) %G, i32 %val, ptr addrspace(3) %L) nounwind !kernel_arg_base_type !20 !arg_type_null_val !21 {
; CHECK: define void @NonKernelFunc4(ptr addrspace(4) noalias %X, ptr addrspace(1) noalias %G, i32 %val, ptr addrspace(3) noalias %L)
  ret void
}

; function has barrier, so we cannot set NoAlias
define void @NonKernelFunc5(ptr addrspace(1) %G, i32 %val, ptr addrspace(3) %L) nounwind !kernel_arg_base_type !22 !arg_type_null_val !23 {
; CHECK: define void @NonKernelFunc5(ptr addrspace(1) %G, i32 %val, ptr addrspace(3) %L)
  call void @_Z7barrierj(i32 1)
  ret void
}

; kernel function is called by KernelFunc and more than one local-mem argument, so we cannot set NoAlias
define void @KernelFuncCallee1(ptr addrspace(1) %G, i32 %val, ptr addrspace(3) %L1, ptr addrspace(3) %L2) nounwind !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_type_qual !4 !kernel_arg_name !5 !kernel_arg_base_type !24 !arg_type_null_val !25 {
; CHECK: define void @KernelFuncCallee1(ptr addrspace(1) noalias %G, i32 %val, ptr addrspace(3) %L1, ptr addrspace(3) %L2)
  ret void
}

; kernel function is called by KernelFunc and has one local-mem argument, so we can set NoAlias
define void @KernelFuncCallee2(ptr addrspace(1) %G, i32 %val, ptr addrspace(3) %L1) nounwind !kernel_arg_addr_space !7 !kernel_arg_access_qual !8 !kernel_arg_type !9 !kernel_arg_type_qual !10 !kernel_arg_name !11 !kernel_arg_base_type !22 !arg_type_null_val !23 {
; CHECK: define void @KernelFuncCallee2(ptr addrspace(1) noalias %G, i32 %val, ptr addrspace(3) noalias %L1)
  ret void
}

; Kernel function has local-mem arguments and is not called by other functions, so we can set NoAlias
define void @KernelFunc(ptr addrspace(1) %G, i32 %val, ptr addrspace(3) %L1, ptr addrspace(3) %L2) nounwind !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_type_qual !4 !kernel_arg_name !5 !kernel_arg_base_type !24 !arg_type_null_val !25 {
; CHECK: define void @KernelFunc(ptr addrspace(1) noalias %G, i32 %val, ptr addrspace(3) noalias %L1, ptr addrspace(3) noalias %L2)
  call void @KernelFuncCallee1(ptr addrspace(1) %G, i32 %val, ptr addrspace(3) %L1, ptr addrspace(3) %L1)
  call void @KernelFuncCallee2(ptr addrspace(1) %G, i32 %val, ptr addrspace(3) %L2)
  ret void
}

declare void @_Z7barrierj(i32)

!sycl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!13}
!opencl.ocl.version = !{!14}
!opencl.used.extensions = !{!15}
!opencl.used.optional.core.features = !{!15}
!opencl.compiler.options = !{!15}

!0 = !{ptr @KernelFuncCallee1, ptr @KernelFuncCallee2, ptr @KernelFunc}
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
!16 = !{!"int*", !"int*", !"int*", !"int*", !"int", !"int*", !"int*"}
!17 = !{ptr addrspace(1) null, ptr addrspace(2) null, ptr addrspace(2) null, ptr addrspace(2) null, i32 0, ptr addrspace(3) null, ptr addrspace(3) null}
!18 = !{!"int*", !"int*", !"int*"}
!19 = !{ptr addrspace(1) null, ptr addrspace(2) null, ptr addrspace(3) null}
!20 = !{!"int*", !"int*", !"int*"}
!21 = !{ptr addrspace(4) null, ptr addrspace(1) null, ptr addrspace(3) null}
!22 = !{!"int*", !"int*"}
!23 = !{ptr addrspace(1) null, ptr addrspace(3) null}
!24 = !{!"int*", !"int*", !"int*"}
!25 = !{ptr addrspace(1) null, ptr addrspace(3) null, ptr addrspace(3) null}

; DEBUGIFY-NOT: WARNING
