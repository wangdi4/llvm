; RUN: opt -passes='debugify,sycl-kernel-sg-emu-loop-construct,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-sg-emu-loop-construct' -S %s | FileCheck %s

; This test checks SGLoopConstruct::hoistSGLIdCalls()
; The non-sync-functions that use get_sub_group_local_id(), will be added by an extra
; %sg.lid input parameter.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK-LABEL: define void @test
define void @test(i32 %a) !kernel_has_sub_groups !1 !sg_emu_size !2 {
sg.loop.exclude:
  br label %entry

entry:
; CHECK: sg.loop.header{{.*}}:
; CHECK: [[SGLID:%.*]] = load i32, ptr {{%sg.lid.ptr.*}}
  call void @dummy_sg_barrier()
  call void @wrapper()
; CHECK: call void @wrapper(i32 [[SGLID]])

  call void @_Z17sub_group_barrierj(i32 1)

  ret void
}

; CHECK-LABEL: define void @non_sync_func(i32 %sg.lid)
define void @non_sync_func() {
entry:
  %a = call i32 @_Z22get_sub_group_local_idv()
  %use = add nuw i32 %a, 1
; CHECK-NOT: call i32 @_Z22get_sub_group_local_idv()
; CHECK: %use = add nuw i32 %sg.lid, 1
  ret void
}

; CHECK-LABEL: define void @wrapper(i32 %sg.lid)
define void @wrapper() {
entry:
  call void @non_sync_func()
; CHECK: call void @non_sync_func(i32 %sg.lid)
  ret void
}

declare i32 @_Z22get_sub_group_local_idv()
declare void @dummy_sg_barrier()
declare void @_Z17sub_group_barrierj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i1 true}
!2 = !{i32 16}

; TODO: The get_sub_group_local_id function will be optimized by this pass. Will
; create tls storage and attach its debug info to the load instruction from
; tls storage.
; DEBUGIFY: WARNING: Missing line

; DEBUGIFY-NOT: WARNING
