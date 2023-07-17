; RUN: opt -passes='debugify,sycl-kernel-sg-emu-loop-construct,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-sg-emu-loop-construct' -S %s | FileCheck %s

; This test checks SGLoopConstruct::collectSyncInsts()
; 1. All subgroup barrier calls should be splitted into a new basic block named
; with prefix "sg.barrier.split."
; 2. All subgroup dummy barrier calls should be splitted into a new basic block
; as well, the label of new BB will be changed to "sg.loop.header.*" via createSGLoop(),
; since any dummy barrier calls is guaranteed to be an entry point of a loop.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test(i32 %a) !kernel_has_sub_groups !1 !sg_emu_size !2 {
sg.loop.exclude:
  br label %entry

; CHECK-LABEL: entry:
; CHECK: br label %[[D1:sg.loop.header.*]]
entry:                                            ; preds = %sg.loop.exclude
; CHECK: [[D1]]:
; CHECK-SAME: ; preds ={{.*}} %entry
; CHECK: call void @something()
  call void @dummy_sg_barrier()
  call void @something()
  br label %sg.barrier.bb.1

sg.barrier.bb.1:                                  ; preds = %entry
; CHECK-LABEL: sg.barrier.split.{{.*}}:
; CHECK: call <16 x i32> @_Z13sub_group_allDv16_iDv16_j
  call void @_Z17sub_group_barrierj(i32 1)
  %call = call <16 x i32> @_Z13sub_group_allDv16_iDv16_j(<16 x i32> undef, <16 x i32> zeroinitializer)
  br label %sg.dummy.bb.

; CHECK-LABEL: sg.dummy.bb.:
; CHECK: br label %[[D2:sg.loop.header.*]]
sg.dummy.bb.:                                     ; preds = %sg.barrier.bb.1
; CHECK: [[D2]]:
; CHECK-SAME: ; preds ={{.*}} %sg.dummy.bb.
; CHECK: call void @something()
  call void @dummy_sg_barrier()
  call void @something()
  br label %sg.barrier.bb.

sg.barrier.bb.:                                   ; preds = %sg.dummy.bb.
; CHECK-LABEL: sg.barrier.split.{{.*}}:
; CHECK: ret void
  call void @_Z17sub_group_barrierj(i32 1)
  ret void
}

declare void @something()
declare void @_Z17sub_group_barrierj(i32)
declare void @dummy_sg_barrier()
declare <16 x i32> @_Z13sub_group_allDv16_iDv16_j(<16 x i32>, <16 x i32>)

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i1 true}
!2 = !{i32 16}

; DEBUGIFY-NOT: WARNING
