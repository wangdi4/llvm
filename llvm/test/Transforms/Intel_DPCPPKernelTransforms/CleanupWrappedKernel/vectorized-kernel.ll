; RUN: opt -dpcpp-kernel-cleanup-wrapped %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -dpcpp-kernel-cleanup-wrapped %s -S | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-cleanup-wrapped %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=dpcpp-kernel-cleanup-wrapped %s -S | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: declare {{.*}} void @__test_separated_args
; CHECK: declare {{.*}} void @____Vectorized_.test_separated_args
; CHECK: define void @test
; CHECK: define void @__Vectorized_.test
; CHECK-NOT: define void @__test_separated_args
; CHECK-NOT: define void @____Vectorized_.test_separated_args

define void @__test_separated_args(i32 addrspace(1)* noalias %dst, i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* noalias %pWorkDim, i64* noalias %pWGId, [4 x i64] %BaseGlbId, i8* noalias %pSpecialBuf, {}* noalias %RuntimeHandle) #0 !kernel_wrapper !{void(i8*, i64*, {}*)* @test} {
  ret void
}

define void @____Vectorized_.test_separated_args(i32 addrspace(1)* noalias %dst, i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* noalias %pWorkDim, i64* noalias %pWGId, [4 x i64] %BaseGlbId, i8* noalias %pSpecialBuf, {}* noalias %RuntimeHandle) #1 !kernel_wrapper !{void(i8*, i64*, {}*)* @__Vectorized_.test} {
  ret void
}

define void @test(i8* noalias %UniformArgs, i64* noalias %pWGId, {}* noalias %RuntimeHandle) {
  ret void
}

define void @__Vectorized_.test(i8* noalias %UniformArgs, i64* noalias %pWGId, {}* noalias %RuntimeHandle) {
  ret void
}

;; TODO: replace "kernel_wrapper" with "kernel-wrapper"
attributes #0 = { alwaysinline "kernel_wrapper"="test" }
attributes #1 = { alwaysinline "kernel_wrapper"="__Vectorized_.test" }

!sycl.kernels = !{!0}
!0 = !{void (i32 addrspace(1)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*, i64*, [4 x i64], i8*, {}*)* @__test_separated_args, void (i32 addrspace(1)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*, i64*, [4 x i64], i8*, {}*)* @____Vectorized_.test_separated_args}

; DEBUGIFY: WARNING: Missing line 1
; DEBUGIFY: WARNING: Missing line 2
; DEBUGIFY: WARNING: Missing variable 1
; DEBUGIFY: WARNING: Missing variable 2
; DEBUGIFY-NOT: WARNING
