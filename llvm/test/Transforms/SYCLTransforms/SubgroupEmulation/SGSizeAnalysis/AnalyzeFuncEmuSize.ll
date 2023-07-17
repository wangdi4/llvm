; RUN: opt -passes="print<sycl-kernel-sg-size-analysis>" %s -S 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK-DAG: Function<test16> Emu Sizes: 16
define void @test16(i32 %a) !kernel_has_sub_groups !1 !sg_emu_size !2 {
entry:
  %call = tail call i32 @foo(i32 %a)
  ret void
}

; CHECK-DAG: Function<test4> Emu Sizes: 4
define void @test4(i32 %a) !kernel_has_sub_groups !1 !sg_emu_size !3 {
entry:
  %call = tail call i32 @foo(i32 %a)
  ret void
}

; CHECK-DAG: Function<test> Emu Sizes: 1
define void @test(i32 %a) !kernel_has_sub_groups !1 !sg_emu_size !4 {
entry:
  %call = tail call i32 @foo(i32 %a)
  ret void
}

; CHECK-DAG: Function<foo> Emu Sizes: 1 4 16
define i32 @foo(i32 %a) {
    ret i32 %a
}

!sycl.kernels = !{!0}

!0 = !{ptr @test16, ptr @test4, ptr @test}
!1 = !{i1 true}
!2 = !{i32 16}
!3 = !{i32 4}
!4 = !{i32 1}
