; RUN: %oclopt -sg-barrier-simplify -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -sg-barrier-simplify -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test1(i32 %a) !kernel_has_sub_groups !1 !sg_emu_size !2 {
; CHECK-LABEL: define void @test1
entry:
; CHECK: call void @dummybarrier.()
  call void @dummy_sg_barrier()
  call void @dummybarrier.()

; no SG barriers or SG dummy barriers in WG dummy region
; CHECK-NOT: call void @_Z17sub_group_barrierj(i32 1)
; CHECK-NOT: call void @dummy_sg_barrier()
  call void @_Z17sub_group_barrierj(i32 1)
  call void @dummy_sg_barrier()

; CHECK: call void @dummybarrier.()
  call void @dummybarrier.()
  ret void
}

define void @test2(i32 %a) !kernel_has_sub_groups !1 !sg_emu_size !2 {
; CHECK-LABEL: define void @test2
entry:
; CHECK: call void @dummybarrier.()
  call void @dummy_sg_barrier()
  call void @dummybarrier.()

; this is not a dummy region
; CHECK: call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z17sub_group_barrierj(i32 1)
  call void @dummy_sg_barrier()

; CHECK: call void @_Z7barrierj(i32 1)
  call void @_Z7barrierj(i32 1)

; this is not a dummy region
; CHECK: call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z17sub_group_barrierj(i32 1)
  call void @dummy_sg_barrier()

; CHECK: call void @dummybarrier.()
  call void @dummybarrier.()
  ret void
}

declare void @dummybarrier.()
declare void @_Z7barrierj(i32)

declare void @dummy_sg_barrier()
declare void @_Z17sub_group_barrierj(i32)

!sycl.kernels = !{!0}

!0 = !{void (i32)* @test1, void (i32)* @test2}
!1 = !{i1 true}
!2 = !{i32 16}

; TODO: The barrier functions will be optimized by this pass. Will evaluate if
; we should keep its debug info or drop it.
; DEBUGIFY-COUNT-6: WARNING: Missing line
; DEBUGIFY-NOT: WARNING
