
; RUN: opt -passes=sycl-kernel-sg-emu-barrier-propagate -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-sg-emu-barrier-propagate -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test(i32 %a) !kernel_has_sub_groups !1 !sg_emu_size !2 {
entry:
; CHECK-LABEL: entry:
; CHECK: call void @dummy_sg_barrier()

  call void @_Z17sub_group_barrierj(i32 1)
; CHECK-NEXT: call void @_Z17sub_group_barrierj(i32 1)
  %cmp = icmp eq i32 %a, 0
  br i1 %cmp, label %is_zero, label %not_zero

is_zero:
; CHECK-LABEL: is_zero:
; CHECK: call void @_Z17sub_group_barrierj(i32 1)
  ret void
; CHECK-NEXT: ret void

not_zero:
; CHECK-LABEL: not_zero:
; CHECK: call void @_Z17sub_group_barrierj(i32 1)
  ret void
; CHECK-NEXT: ret void
}

declare void @_Z17sub_group_barrierj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i1 true}
!2 = !{i32 16}

; dummy_sg_barrier is emitted by barrier pass and will be removed later, so it's
; not needed to attach debug location for it.
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- call void @dummy_sg_barrier()
; DEBUGIFY-NOT: WARNING
