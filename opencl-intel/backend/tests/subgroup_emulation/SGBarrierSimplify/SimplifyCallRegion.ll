; RUN: %oclopt -sg-barrier-simplify -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @foo() {
call void @dummybarrier.()
call void @dummy_sg_barrier()
call void @_Z17sub_group_barrierj(i32 1)
call void @_Z7barrierj(i32 1)
call void @dummy_sg_barrier()
call void @_Z17sub_group_barrierj(i32 1)
ret void
}

define void @test(i32 %a) !kernel_has_sub_groups !1 !sg_emu_size !2 {
; CHECK-LABEL: define void @test
; CHECK: call void @_Z7barrierj(i32 1)
; CHECK-NOT: call void @dummy_sg_barrier()
; CHECK-NOT: call void @_Z17sub_group_barrierj(i32 1)
; CHECK: call void @dummybarrier.()
entry:
  call void @_Z7barrierj(i32 1)
  call void @dummy_sg_barrier()
  call void @_Z17sub_group_barrierj(i32 1)
  call void @foo()
  call void @dummy_sg_barrier()
  call void @dummybarrier.()
  ret void
}


declare void @dummybarrier.()
declare void @_Z7barrierj(i32)

declare void @dummy_sg_barrier()
declare void @_Z17sub_group_barrierj(i32)

!opencl.kernels = !{!0}

!0 = !{void (i32)* @test}
!1 = !{i1 true}
!2 = !{i32 16}
