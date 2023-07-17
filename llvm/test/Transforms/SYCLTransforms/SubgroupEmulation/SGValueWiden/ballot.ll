; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s | FileCheck %s

; CHECK: call <32 x i32> @_Z16sub_group_ballotDv8_iDv8_j(<8 x i32> {{.*}}, <8 x i32> {{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test() !kernel_has_sub_groups !1 !sg_emu_size !2 {
sg.barrier.bb:
  %call.i.i = call <4 x i32> @_Z16sub_group_balloti(i32 0)
  br label %sg.dummy.bb

sg.dummy.bb:                                    ; preds = %sg.barrier.bb
  call void @dummy_sg_barrier()
  %vecext.i.i = extractelement <4 x i32> %call.i.i, i32 0
  ret void
}

declare <4 x i32> @_Z16sub_group_balloti(i32) #0

declare void @dummy_sg_barrier()

attributes #0 = { "vector-variants"="_ZGVbM8v__Z16sub_group_balloti(_Z16sub_group_ballotDv8_iDv8_j)" }

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i1 true}
!2 = !{i32 8}

; DEBUGIFY-NOT: WARNING
