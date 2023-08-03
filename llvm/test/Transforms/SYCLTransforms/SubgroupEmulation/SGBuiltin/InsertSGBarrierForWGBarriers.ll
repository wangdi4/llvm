; RUN: opt -sycl-vect-info=%p/../../Inputs/VectInfo64.gen -passes='debugify,sycl-kernel-sg-emu-builtin,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -sycl-vect-info=%p/../../Inputs/VectInfo64.gen -passes='sycl-kernel-sg-emu-builtin' -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test(i32 %a) !kernel_has_sub_groups !1 !sg_emu_size !2 {
entry:
  call void @dummy_barrier.()
; CHECK: call void @dummy_barrier.()
; CHECK-NEXT: call void @dummy_sg_barrier()

; CHECK: call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z7barrierj(i32 1)
; CHECK-NEXT: call void @_Z7barrierj(i32 1)
; CHECK-NEXT: call void @dummy_sg_barrier()
  ret void
}

declare void @dummy_barrier.()
declare void @_Z7barrierj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i1 true}
!2 = !{i32 16}

; dummy_sg_barrier is emitted by barrier pass and will be removed later, so it's
; not needed to attach debug location for it.
; DEBUGIFY-COUNT-2: WARNING: Instruction with empty DebugLoc in function test -- call void @dummy_sg_barrier()
; DEBUGIFY-NOT: WARNING
