; RUN: opt -passes=sycl-kernel-cleanup-wrapped %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-cleanup-wrapped %s -S | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: declare {{.*}} void @__test_separated_args
; CHECK: declare {{.*}} void @____Vectorized_.test_separated_args
; CHECK: define void @test
; CHECK: define void @__Vectorized_.test
; CHECK-NOT: define void @__test_separated_args
; CHECK-NOT: define void @____Vectorized_.test_separated_args

define void @__test_separated_args(ptr addrspace(1) noalias %dst, ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle) #0 !kernel_wrapper !{ptr @test} !kernel_arg_base_type !1 !arg_type_null_val !2 {
  ret void
}

define void @____Vectorized_.test_separated_args(ptr addrspace(1) noalias %dst, ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle) #1 !kernel_wrapper !{ptr @__Vectorized_.test} !kernel_arg_base_type !1 !arg_type_null_val !2 {
  ret void
}

define void @test(ptr noalias %UniformArgs, ptr noalias %pWGId, ptr noalias %RuntimeHandle) {
  ret void
}

define void @__Vectorized_.test(ptr noalias %UniformArgs, ptr noalias %pWGId, ptr noalias %RuntimeHandle) {
  ret void
}

;; TODO: replace "kernel_wrapper" with "kernel-wrapper"
attributes #0 = { alwaysinline "kernel_wrapper"="test" }
attributes #1 = { alwaysinline "kernel_wrapper"="__Vectorized_.test" }

!sycl.kernels = !{!0}
!0 = !{ptr @__test_separated_args, ptr @____Vectorized_.test_separated_args}
!1 = !{!"int*"}
!2 = !{ptr addrspace(1) null}

; DEBUGIFY: WARNING: Missing line 1
; DEBUGIFY: WARNING: Missing line 2
; DEBUGIFY: WARNING: Missing variable 1
; DEBUGIFY: WARNING: Missing variable 2
; DEBUGIFY-NOT: WARNING
