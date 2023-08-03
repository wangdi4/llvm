; RUN: opt -passes=sycl-kernel-resolve-wi-call -S %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-resolve-wi-call -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare void @_Z8prefetchPU3AS1Kcm(ptr addrspace(1) noundef, i64 noundef)
declare void @_Z8prefetchPU3AS1KDv16_dm(ptr addrspace(1) noundef, i64 noundef)
declare void @_Z8prefetchPU3AS1Klm(ptr addrspace(1) noundef, i64 noundef)
declare void @_Z8prefetchPU3AS1KDv3_sm(ptr addrspace(1) noundef, i64 noundef)

define void @test_arg(ptr addrspace(1) noundef align 1 %src, ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle) {
entry:
; CHECK-LABEL: define void @test_arg(
; CHECK: call void @__lprefetch(ptr {{.*}}, i64 0, i64 1)

  tail call void @_Z8prefetchPU3AS1Kcm(ptr addrspace(1) %src, i64 0)
  ret void
}

define void @test_add(ptr addrspace(1) noundef align 4 %src, i64 %idx, ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle) {
entry:
; CHECK-LABEL: define void @test_add(
; CHECK: call void @__lprefetch(ptr {{.*}}, i64 0, i64 128)

  %add.ptr = getelementptr inbounds i32, ptr addrspace(1) %src, i64 %idx
  tail call void @_Z8prefetchPU3AS1KDv16_dm(ptr addrspace(1) %add.ptr, i64 0)
  ret void
}

define void @test_vector(ptr addrspace(1) noundef align 8 %src, <16 x ptr addrspace(1)> %splat, ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle) {
entry:
; CHECK-LABEL: define void @test_vector(
; CHECK: call void @__lprefetch(ptr {{.*}}, i64 0, i64 8)

  %vectorGEP = getelementptr inbounds i64, <16 x ptr addrspace(1)> %splat, <16 x i64> zeroinitializer
  %extract = extractelement <16 x ptr addrspace(1)> %vectorGEP, i64 0
  tail call void @_Z8prefetchPU3AS1Klm(ptr addrspace(1) %extract, i64 0)
  ret void
}

define void @test_nonkernel(ptr addrspace(1) noundef %src, ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle) {
entry:
; CHECK-LABEL: define void @test_nonkernel(
; CHECK: call void @__lprefetch(ptr {{.*}}, i64 0, i64 8)

  tail call void @_Z8prefetchPU3AS1KDv3_sm(ptr addrspace(1) %src, i64 0)
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @test_arg, ptr @test_add, ptr @test_vector}

; DEBUGIFY-NOT: WARNING
