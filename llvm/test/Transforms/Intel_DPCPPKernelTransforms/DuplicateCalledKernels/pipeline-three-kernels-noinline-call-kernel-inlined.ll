; RUN: opt -passes=dpcpp-kernel-duplicate-called-kernels,dpcpp-kernel-add-implicit-args,dpcpp-kernel-local-buffers -S --disable-output %s

; Check that assert in LocalBufferInfoImpl::computeLocalsSizeOffset() isn't
; triggered.
; This file differs from three-kernels-noinline-call-kernel.ll in that a kernel
; is inlined.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@test.i = internal addrspace(3) global [100 x i32] undef, align 4
@test.j = internal addrspace(3) global i32 undef, align 4
@test3.s = internal addrspace(3) global [10 x double] undef, align 8
@test3.t = internal addrspace(3) global double undef, align 8

define internal fastcc zeroext i1 @foo() unnamed_addr {
entry:
  %0 = load i32, i32 addrspace(3)* getelementptr inbounds ([100 x i32], [100 x i32] addrspace(3)* @test.i, i64 0, i64 0), align 4
  %1 = load i32, i32 addrspace(3)* @test.j, align 4
  ret i1 0
}

define dso_local void @test(i32 addrspace(1)* noalias noundef align 4 %results) {
entry:
  store i32 0, i32 addrspace(3)* getelementptr inbounds ([100 x i32], [100 x i32] addrspace(3)* @test.i, i64 0, i64 0), align 4
  store i32 0, i32 addrspace(3)* @test.j, align 4
  %call2 = tail call fastcc zeroext i1 @foo()
  ret void
}

define internal fastcc void @xyz(i32 addrspace(1)* noalias noundef %results) unnamed_addr {
entry:
  store i32 0, i32 addrspace(3)* getelementptr inbounds ([100 x i32], [100 x i32] addrspace(3)* @test.i, i64 0, i64 0), align 4
  store i32 1, i32 addrspace(3)* @test.j, align 4
  %call2.i = tail call fastcc zeroext i1 @foo()
  ret void
}

define dso_local void @test2(i32 addrspace(1)* noalias noundef align 4 %results) local_unnamed_addr {
entry:
  tail call fastcc void @xyz(i32 addrspace(1)* noundef %results)
  ret void
}

define internal fastcc zeroext i1 @bar() unnamed_addr {
entry:
  %0 = load double, double addrspace(3)* getelementptr inbounds ([10 x double], [10 x double] addrspace(3)* @test3.s, i64 0, i64 0), align 8
  %1 = load double, double addrspace(3)* @test3.t, align 8
  ret i1 0
}

define dso_local void @test3(i32 addrspace(1)* noalias noundef align 4 %results) local_unnamed_addr {
entry:
  store double 0.000000e+00, double addrspace(3)* getelementptr inbounds ([10 x double], [10 x double] addrspace(3)* @test3.s, i64 0, i64 0), align 8
  store double 0.000000e+00, double addrspace(3)* @test3.t, align 8
  %call4 = tail call fastcc zeroext i1 @bar()
  tail call fastcc void @xyz(i32 addrspace(1)* noundef %results)
  ret void
}

!sycl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*)* @test, void (i32 addrspace(1)*)* @test2, void (i32 addrspace(1)*)* @test3}
