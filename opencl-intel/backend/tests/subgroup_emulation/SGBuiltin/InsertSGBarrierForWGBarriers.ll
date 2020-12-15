; RUN: %oclopt -sg-emu-builtin -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test(i32 %a) !kernel_has_sub_groups !1 !sg_emu_size !2 {
entry:
; CHECK: call void @dummy_sg_barrier()
  call void @dummybarrier.()
; CHECK-NEXT: call void @dummybarrier.()
; CHECK-NEXT: call void @dummy_sg_barrier()

; CHECK: call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z7barrierj(i32 1)
; CHECK-NEXT: call void @_Z7barrierj(i32 1)
; CHECK-NEXT: call void @dummy_sg_barrier()
  ret void
}

declare void @dummybarrier.()
declare void @_Z7barrierj(i32)

!opencl.kernels = !{!0}

!0 = !{void (i32)* @test}
!1 = !{i1 true}
!2 = !{i32 16}
