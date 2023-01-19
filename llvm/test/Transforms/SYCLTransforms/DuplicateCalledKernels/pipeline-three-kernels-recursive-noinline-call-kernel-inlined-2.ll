; RUN: opt -passes=dpcpp-kernel-duplicate-called-kernels,dpcpp-kernel-add-implicit-args,dpcpp-kernel-local-buffers -S --disable-output %s

; Check that assert in LocalBufferInfoImpl::computeLocalsSizeOffset() isn't
; triggered.
; This file differs from three-kernels-recursive-noinline-call-kernel.ll in
; that a kernel is inlined.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@test.j = internal unnamed_addr addrspace(3) global i32 undef, align 4

define dso_local void @test(i32 addrspace(1)* noundef align 4 %results) local_unnamed_addr {
entry:
  %0 = load i32, i32 addrspace(1)* %results, align 4
  tail call void @_Z7barrierj(i32 noundef 1)
  %1 = load i32, i32 addrspace(3)* @test.j, align 4
  ret void
}

declare void @_Z7barrierj(i32 noundef) local_unnamed_addr

define internal fastcc void @xyz(i32 addrspace(1)* noundef %results) unnamed_addr !recursive_call !1 {
entry:
  tail call fastcc void @xyz(i32 addrspace(1)* noundef %results)
  store i32 0, i32 addrspace(3)* @test.j, align 4
  tail call void @_Z7barrierj(i32 noundef 1)
  %0 = load i32, i32 addrspace(3)* @test.j, align 4
  ret void
}

define dso_local void @test2(i32 addrspace(1)* noundef align 4 %results) local_unnamed_addr !recursive_call !1 {
entry:
  tail call fastcc void @xyz(i32 addrspace(1)* noundef %results)
  ret void
}

define dso_local void @test3(i32 addrspace(1)* noundef align 4 %results) local_unnamed_addr !recursive_call !1 {
entry:
  tail call fastcc void @xyz(i32 addrspace(1)* noundef %results)
  ret void
}

!sycl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*)* @test, void (i32 addrspace(1)*)* @test2, void (i32 addrspace(1)*)* @test3}
!1 = !{i1 true}
