; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test() !kernel_has_sub_groups !1 !sg_emu_size !2 {
entry:
  call void @dummybarrier.()
  br label %sg.dummy.bb.

sg.dummy.bb.:                                     ; preds = %entry
  call void @dummy_sg_barrier()
  %x = alloca i32, align 4
  %a = alloca i32, align 4
  %call = call i64 @_Z12get_local_idj(i32 0)
  %conv = trunc i64 %call to i32
  store i32 %conv, ptr %x, align 4
  %0 = load i32, ptr %x, align 4
  br label %sg.barrier.bb.1

; Currently, this case is identical to UniformArg.ll
; CHECK-LABEL: sg.barrier.bb.1:
; CHECK: %[[#OP1:]] = load <16 x i32>, ptr %w., align 64
; CHECK: call <16 x i32> @_Z13sub_group_allDv16_iDv16_j(<16 x i32> %[[#OP1]], <16 x i32> %mask.i32)
sg.barrier.bb.1:                                  ; preds = %sg.dummy.bb.
  call void @_Z17sub_group_barrierj(i32 1)
  %call1 = call i32 @_Z13sub_group_alli(i32 %0)
  br label %sg.dummy.bb.3

sg.dummy.bb.3:                                    ; preds = %sg.barrier.bb.1
  call void @dummy_sg_barrier()
  store i32 %call1, ptr %a, align 4
  br label %sg.barrier.bb.

sg.barrier.bb.:                                   ; preds = %sg.dummy.bb.3
  call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z7barrierj(i32 1)
  br label %sg.dummy.bb.2

sg.dummy.bb.2:                                    ; preds = %sg.barrier.bb.
  call void @dummy_sg_barrier()
  ret void
}

declare i64 @_Z12get_local_idj(i32)
declare i32 @_Z13sub_group_alli(i32) #0

declare void @dummybarrier.()
declare void @_Z7barrierj(i32)

declare void @_Z17sub_group_barrierj(i32)
declare void @dummy_sg_barrier()

attributes #0 = { "vector-variants"="_ZGVbM16v__Z13sub_group_alli(_Z13sub_group_allDv16_iDv16_j)" }

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i1 true}
!2 = !{i32 16}

; DEBUGIFY-NOT: WARNING
; FIXME: SGValueWiden does not respect llvm.dbg.value and llvm.dbg.addr
; DEBUGIFY-COUNT-2: WARNING: Missing line
; DEBUGIFY-NOT: WARNING
