; RUN: opt -passes=sycl-kernel-sg-emu-barrier-simplify -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-sg-emu-barrier-simplify -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test1(i32 %a) !kernel_has_sub_groups !1 !sg_emu_size !2 {
; CHECK-LABEL: define void @test1
entry:
; CHECK: entry:
; CHECK-NOT: call void @dummy_sg_barrier()
; CHECK-NEXT: call void @dummy_barrier.()
  call void @dummy_sg_barrier()
  call void @dummy_sg_barrier()
  call void @dummy_sg_barrier()
  call void @dummy_barrier.()

; CHECK: call void @_Z17sub_group_barrierj(i32 1)
; CHECK-NOT: call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z17sub_group_barrierj(i32 1)

; CHECK: call void @dummy_sg_barrier()
  call void @dummy_sg_barrier()
  ret void
}

define void @test2(i32 %a) !kernel_has_sub_groups !1 !sg_emu_size !2 {
; CHECK-LABEL: define void @test2
entry:
; CHECK: entry:
; CHECK-NEXT: call void @dummy_sg_barrier()
; CHECK-NOT: call void @dummy_sg_barrier()
; CHECK-NOT: call void @_Z17sub_group_barrierj(i32 1)
  call void @dummy_sg_barrier()
  call void @dummy_sg_barrier()
  call void @_Z17sub_group_barrierj(i32 1)

  call void @dummy_sg_barrier()
  call void @_Z17sub_group_barrierj(i32 1)

; CHECK-LABEL: call void @anything()
  call void @anything()

; CHECK: call void @dummy_sg_barrier()
; CHECK-NOT: call void @_Z17sub_group_barrierj(i32 1)
  call void @dummy_sg_barrier()
  call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z17sub_group_barrierj(i32 1)

; CHECK-NOT: call void @dummy_sg_barrier()
  call void @dummy_sg_barrier()
  ret void
}

declare void @dummy_barrier.()
declare void @anything()

declare void @dummy_sg_barrier()
declare void @_Z17sub_group_barrierj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @test1, ptr @test2}
!1 = !{i1 true}
!2 = !{i32 16}

; TODO: The barrier functions will be optimized out by this pass. Will evaluate
; if we should keep its debug info or drop it.
; DEBUGIFY-COUNT-13: WARNING: Missing line
; DEBUGIFY-NOT: WARNING
