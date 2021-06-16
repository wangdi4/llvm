; RUN: %oclopt -cleanup-wrapped-kernels -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -cleanup-wrapped-kernels -S < %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: declare !kernel_wrapper {{![0-9]}} !vectorized_kernel {{![0-9]}} void @__test_separated_args
; CHECK: declare !kernel_wrapper {{![0-9]}} !scalarized_kernel {{![0-9]}} void @____Vectorized_.test_separated_args
; CHECK: define void @test
; CHECK: define void @__Vectorized_.test
; CHECK-NOT: define void @__test_separated_args
; CHECK-NOT: define void @____Vectorized_.test_separated_args

define void @__test_separated_args(i32 addrspace(1)* noalias %dst, i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* noalias %pWorkDim, i64* noalias %pWGId, [4 x i64] %BaseGlbId, i8* noalias %pSpecialBuf, {}* noalias %RuntimeHandle) !kernel_wrapper !2 !vectorized_kernel !1 {
  ret void
}

define void @____Vectorized_.test_separated_args(i32 addrspace(1)* noalias %dst, i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* noalias %pWorkDim, i64* noalias %pWGId, [4 x i64] %BaseGlbId, i8* noalias %pSpecialBuf, {}* noalias %RuntimeHandle) !kernel_wrapper !3 !scalarized_kernel !0 {
  ret void
}

define void @test(i8* noalias %pUniformArgs, i64* noalias %pWGId, {}* noalias %RuntimeHandle) !vectorized_kernel !1 {
  ret void
}

define void @__Vectorized_.test(i8* noalias %pUniformArgs, i64* noalias %pWGId, {}* noalias %RuntimeHandle) !scalarized_kernel !0 {
  ret void
}

!opencl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @__test_separated_args}
!1 = !{void (i32 addrspace(1)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @____Vectorized_.test_separated_args}
!2 = !{void (i8*, i64*, {}*)* @test}
!3 = !{void (i8*, i64*, {}*)* @__Vectorized_.test}

; DEBUGIFY-NOT: WARNING
; __test_separated_args and ____Vectorized_.test_separated_args are wrapped, and will be removed in the pass, we ignore related warnings
; DEBUGIFY: WARNING: Missing line 1
; DEBUGIFY: WARNING: Missing line 2
; DEBUGIFY: WARNING: Missing variable 1
; DEBUGIFY: WARNING: Missing variable 2
; DEBUGIFY-NOT: WARNING
