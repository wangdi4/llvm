; RUN: %oclopt -sg-barrier-propagate -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test(i32 %a) !kernel_has_sub_groups !1 !sg_emu_size !2 {
; CHECK-LABEL: define void @test
entry:
; CHECK: entry:
; CHECK-NEXT: call void @dummy_sg_barrier()

; CHECK: call void @_Z17sub_group_barrierj(i32 1)
  call void @direct_sync_func(i32 1)
; CHECK-NEXT: call void @direct_sync_func(i32 1)
; CHECK-NEXT: call void @dummy_sg_barrier()

; CHECK: call void @_Z17sub_group_barrierj(i32 1)
  ret void
; CHECK-NEXT: ret void
}

define void @direct_sync_func(i32 %a) {
; CHECK-LABEL: define void @direct_sync_func
entry:
  call void @_Z17sub_group_barrierj(i32 1)
  ret void
}

define void @foo() {
; CHECK-LABEL: define void @foo
entry:
; CHECK-NOT: call void @_Z17sub_group_barrierj(i32 1)
  call void @direct_sync_func(i32 1)
; CHECK: call void @direct_sync_func(i32 1)
; CHECK-NOT: call void @dummy_sg_barrier()
  ret void
}

declare void @_Z17sub_group_barrierj(i32)

!opencl.kernels = !{!0}

!0 = !{void (i32)* @test}
!1 = !{i1 true}
!2 = !{i32 16}
