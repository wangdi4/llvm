; RUN: opt -passes=sycl-kernel-sg-emu-barrier-simplify -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-sg-emu-barrier-simplify -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test(i32 %a) !kernel_has_sub_groups !1 !sg_emu_size !2 {
entry:
; CHECK-LABEL: entry:
; CHECK-NEXT: call void @dummy_sg_barrier()
  call void @dummy_sg_barrier()
  br label %dummy_first

dummy_first:
; CHECK-LABEL: dummy_first:
; CHECK-SAME: preds = %entry
; CHECK-NEXT: call void @dummy_sg_barrier()
  call void @dummy_sg_barrier()
  call void @something()

; CHECK: [[LABEL1:.*]]:
; CHECK-SAME: preds = %dummy_first
; CHECK: call void @_Z17sub_group_barrierj(i32 1)
; CHECK: call void @something()
  call void @_Z17sub_group_barrierj(i32 1)
  call void @something()

; CHECK: [[LABEL2:.*]]:
; CHECK-SAME: preds = %[[LABEL1]]
  call void @dummy_sg_barrier()
  br label %barrier_first

barrier_first:
; CHECK-LABEL: barrier_first:
; CHECK-SAME: preds = %[[LABEL2]]
; CHECK-NEXT: call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z17sub_group_barrierj(i32 1)
  call void @something()

; CHECK: [[LABEL3:.*]]:
; CHECK-SAME: preds = %barrier_first
; CHECK: call void @dummy_sg_barrier()
; CHECK: call void @something()
  call void @dummy_sg_barrier()
  call void @something()

; CHECK: [[LABEL4:.*]]:
; CHECK-SAME: preds = %[[LABEL3]]
; CHECK: call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z17sub_group_barrierj(i32 1)

  ret void
}

declare void @something()

declare void @dummy_sg_barrier()
declare void @_Z17sub_group_barrierj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i1 true}
!2 = !{i32 16}

; DEBUGIFY-NOT: WARNING
