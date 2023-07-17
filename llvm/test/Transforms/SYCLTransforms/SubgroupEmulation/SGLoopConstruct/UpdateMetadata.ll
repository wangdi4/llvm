; RUN: opt -passes='debugify,sycl-kernel-sg-emu-loop-construct,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-sg-emu-loop-construct' -S %s | FileCheck %s

; This test checks SGLoopConstruct::updateMetadata()
; 1. For emulated kernels, vectorized_width is set as sg_emu_size (the first one),
; vectorization_dimension is set as 0,
; sg_emu_size metadata is removed.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: define void @test
; CHECK-NOT: !sg_emu_size
; CHECK-DAG: !vectorized_width ![[#VW:]]
; CHECK-DAG: !vectorization_dimension ![[#VD:]]
; CHECK-NOT: !sg_emu_size
define void @test(i32 %a) !kernel_has_sub_groups !1 !sg_emu_size !2 {
sg.loop.exclude:
  br label %entry

entry:                                            ; preds = %sg.loop.exclude
  call void @dummy_sg_barrier()
  call void @something()
  br label %sg.barrier.bb.1

sg.barrier.bb.1:                                  ; preds = %entry
  call void @_Z17sub_group_barrierj(i32 1)
  %call = call <16 x i32> @_Z13sub_group_allDv16_iDv16_j(<16 x i32> undef, <16 x i32> zeroinitializer)
  br label %sg.dummy.bb.

sg.dummy.bb.:                                     ; preds = %sg.barrier.bb.1
  call void @dummy_sg_barrier()
  call void @something()
  br label %sg.barrier.bb.

sg.barrier.bb.:                                   ; preds = %sg.dummy.bb.
  call void @_Z17sub_group_barrierj(i32 1)
  ret void
}

declare void @something()
declare void @_Z17sub_group_barrierj(i32)
declare void @dummy_sg_barrier()
declare <16 x i32> @_Z13sub_group_allDv16_iDv16_j(<16 x i32>, <16 x i32>)

; CHECK: !sycl.kernels
!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i1 true}
!2 = !{i32 16}
; CHECK-DAG: ![[#VW]] = !{i32 16}
; CHECK-DAG: ![[#VD]] = !{i32 0}

; DEBUGIFY-NOT: WARNING
