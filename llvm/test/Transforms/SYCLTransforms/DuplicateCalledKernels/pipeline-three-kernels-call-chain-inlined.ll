; RUN: opt -passes=sycl-kernel-duplicate-called-kernels,sycl-kernel-add-implicit-args,sycl-kernel-local-buffers -S --disable-output %s

; Check that assert in LocalBufferInfoImpl::computeLocalsSizeOffset() isn't
; triggered.
; This file differs from three-kernels-call-chain.ll in that a kernel is
; inlined.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@test.i = internal addrspace(3) global [100 x i32] undef, align 4
@test.j = internal addrspace(3) global i32 undef, align 4
@test3.s = internal addrspace(3) global [10 x double] undef, align 8
@test3.t = internal addrspace(3) global double undef, align 8

define internal fastcc zeroext i1 @foo() unnamed_addr {
entry:
  %0 = load i32, ptr addrspace(3) getelementptr inbounds ([100 x i32], ptr addrspace(3) @test.i, i64 0, i64 1), align 4
  %1 = load i32, ptr addrspace(3) @test.j, align 4
  ret i1 false
}

define dso_local void @test(ptr addrspace(1) noalias noundef align 4 %results) local_unnamed_addr !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  store i32 0, ptr addrspace(3) getelementptr inbounds ([100 x i32], ptr addrspace(3) @test.i, i64 0, i64 1), align 4
  store i32 1, ptr addrspace(3) @test.j, align 4
  %call2 = tail call fastcc zeroext i1 @foo()
  ret void
}

define dso_local void @test2(ptr addrspace(1) noalias noundef align 4 %results) local_unnamed_addr !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  store i32 0, ptr addrspace(3) getelementptr inbounds ([100 x i32], ptr addrspace(3) @test.i, i64 0, i64 1), align 4
  store i32 1, ptr addrspace(3) @test.j, align 4
  %call2.i = tail call fastcc zeroext i1 @foo()
  ret void
}

define internal fastcc zeroext i1 @bar() unnamed_addr {
entry:
  %0 = load double, ptr addrspace(3) getelementptr inbounds ([10 x double], ptr addrspace(3) @test3.s, i64 0, i64 1), align 8
  %1 = load double, ptr addrspace(3) @test3.t, align 8
  ret i1 false
}

define dso_local void @test3(ptr addrspace(1) noalias noundef align 4 %results) local_unnamed_addr !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  store double 0.000000e+00, ptr addrspace(3) getelementptr inbounds ([10 x double], ptr addrspace(3) @test3.s, i64 0, i64 1), align 8
  store double 0.000000e+00, ptr addrspace(3) @test3.t, align 8
  %call4 = tail call fastcc zeroext i1 @bar()
  store i32 2, ptr addrspace(3) getelementptr inbounds ([100 x i32], ptr addrspace(3) @test.i, i64 0, i64 1), align 4
  store i32 3, ptr addrspace(3) @test.j, align 4
  %call2.i.i = tail call fastcc zeroext i1 @foo()
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @test, ptr @test2, ptr @test3}
!1 = !{!"int*"}
!2 = !{ptr addrspace(1) null}
