
; RUN: opt -passes=sycl-kernel-sg-emu-barrier-propagate -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-sg-emu-barrier-propagate -S %s | FileCheck %s

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

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i1 true}
!2 = !{i32 16}

; dummy_sg_barrier is emitted by barrier pass and will be removed later, so it's
; not needed to attach debug location for it.
; DEBUGIFY-COUNT-3: WARNING: Instruction with empty DebugLoc in function {{.*}} -- call void @dummy_sg_barrier()
; DEBUGIFY-NOT: WARNING
