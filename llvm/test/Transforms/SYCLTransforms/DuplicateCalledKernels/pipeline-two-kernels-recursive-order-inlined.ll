; RUN: opt -passes=sycl-kernel-duplicate-called-kernels,sycl-kernel-add-implicit-args,sycl-kernel-local-buffers -S --disable-output %s

; Check that assert in LocalBufferInfoImpl::computeLocalsSizeOffset() isn't
; triggered.
; This file differs from two-kernels-recursive-order.ll in that a kernel is
; inlined.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@test.i = internal addrspace(3) global [100 x i32] undef, align 4
@test.k = internal addrspace(3) global i32 undef, align 4
@test.j = internal addrspace(3) global i32 undef, align 4
@test2.i = internal addrspace(3) global [100 x i32] undef, align 4
@test2.j = internal addrspace(3) global i32 undef, align 4

define internal fastcc i32 @yyy() unnamed_addr !recursive_call !1 {
entry:
  %0 = load i32, ptr addrspace(3) getelementptr inbounds ([100 x i32], ptr addrspace(3) @test.i, i64 0, i64 1), align 4
  %call = tail call fastcc i32 @foo()
  ret i32 0
}

define internal fastcc i32 @foo() unnamed_addr !recursive_call !1 {
entry:
  %0 = load i32, ptr addrspace(3) getelementptr inbounds ([100 x i32], ptr addrspace(3) @test.i, i64 0, i64 1), align 4
  %call = tail call fastcc i32 @xxx()
  %add = add nsw i32 %call, %0
  ret i32 %add
}

define internal fastcc i32 @xxx() unnamed_addr !recursive_call !1 {
entry:
  %0 = load i32, ptr addrspace(3) @test.k, align 4
  %call = tail call fastcc i32 @yyy()
  %add = add nsw i32 %call, %0
  ret i32 %add
}

define internal fastcc i32 @bar() unnamed_addr !recursive_call !1 {
entry:
  %0 = load i32, ptr addrspace(3) @test.j, align 4
  %call = tail call fastcc i32 @foo()
  %add = add nsw i32 %call, %0
  ret i32 %add
}

define dso_local void @test(ptr addrspace(1) noalias noundef align 4 %results) local_unnamed_addr !recursive_call !1 !kernel_arg_base_type !2 !arg_type_null_val !3 {
entry:
  store i32 0, ptr addrspace(3) getelementptr inbounds ([100 x i32], ptr addrspace(3) @test.i, i64 0, i64 1), align 4
  store i32 0, ptr addrspace(3) @test.j, align 4
  store i32 0, ptr addrspace(3) @test.k, align 4
  %call1 = tail call fastcc i32 @bar()
  ret void
}

define internal fastcc i32 @zzz() unnamed_addr {
entry:
  %0 = load i32, ptr addrspace(3) getelementptr inbounds ([100 x i32], ptr addrspace(3) @test2.i, i64 0, i64 1), align 4
  %1 = load i32, ptr addrspace(3) @test2.j, align 4
  ret i32 0
}

define dso_local void @test2(ptr addrspace(1) noalias noundef align 4 %results) local_unnamed_addr !recursive_call !1 !kernel_arg_base_type !2 !arg_type_null_val !3 {
entry:
  %call1 = tail call fastcc i32 @zzz()
  store i32 0, ptr addrspace(3) getelementptr inbounds ([100 x i32], ptr addrspace(3) @test.i, i64 0, i64 1), align 4
  store i32 0, ptr addrspace(3) @test.j, align 4
  store i32 0, ptr addrspace(3) @test.k, align 4
  %call1.i = tail call fastcc i32 @bar()
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @test, ptr @test2}
!1 = !{i1 true}
!2 = !{!"int*"}
!3 = !{ptr addrspace(1) null}
