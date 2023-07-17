; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test(i32 %x) !kernel_has_sub_groups !1 !sg_emu_size !2 {

; CHECK-LABEL: sg.loop.exclude:
; CHECK: %w.x.addr = alloca <16 x i32>, align 64
; CHECK-NEXT: %w.a = alloca <16 x i32>, align 64
; CHECK-NEXT: %w. = alloca <16 x i32>, align 64
; CHECK-NEXT: %u.call = alloca i32, align 4
entry:
  call void @dummybarrier.()
  br label %sg.dummy.bb.

; CHECK-LABEL: sg.dummy.bb.:
; CHECK:  %sg.lid.1 = call i32 @_Z22get_sub_group_local_idv()
; CHECK-NEXT: %[[#OP1:]] = getelementptr <16 x i32>, ptr %w.x.addr, i32 0, i32 %sg.lid.1
; CHECK-NEXT: store i32 %x, ptr %[[#OP1]], align 4
; CHECK-NEXT: %sg.lid. = call i32 @_Z22get_sub_group_local_idv()
; CHECK-NEXT: %[[#OP2:]] = getelementptr <16 x i32>, ptr %w.x.addr, i32 0, i32 %sg.lid.
; CHECK-NEXT: %[[#OP3:]] = load i32, ptr %[[#OP2]], align 4
; CHECK-NEXT: %sg.lid.3 = call i32 @_Z22get_sub_group_local_idv()
; CHECK-NEXT: %[[#OP4:]] = getelementptr <16 x i32>, ptr %w., i32 0, i32 %sg.lid.3
; CHECK-NEXT: store i32 %[[#OP3]], ptr %[[#OP4]], align 4
sg.dummy.bb.:                                     ; preds = %entry
  call void @dummy_sg_barrier()
  %x.addr = alloca i32, align 4
  %a = alloca i32, align 4
  store i32 %x, ptr %x.addr, align 4
  %0 = load i32, ptr %x.addr, align 4
  br label %sg.barrier.bb.1

; CHECK-LABEL: sg.barrier.bb.1:
; CHECK: %[[#OP5:]] = load <16 x i32>, ptr %w., align 64
; CHECK: call <16 x i32> @_Z13sub_group_allDv16_iDv16_j(<16 x i32> %[[#OP5]], <16 x i32> %mask.i32)
sg.barrier.bb.1:                                  ; preds = %sg.dummy.bb.
  call void @_Z17sub_group_barrierj(i32 1)
  %call = call i32 @_Z13sub_group_alli(i32 %0)
  br label %sg.dummy.bb.3

sg.dummy.bb.3:                                    ; preds = %sg.barrier.bb.1
  call void @dummy_sg_barrier()
  store i32 %call, ptr %a, align 4
  br label %sg.barrier.bb.

sg.barrier.bb.:                                   ; preds = %sg.dummy.bb.3
  call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z7barrierj(i32 1)
  br label %sg.dummy.bb.2

sg.dummy.bb.2:                                    ; preds = %sg.barrier.bb.
  call void @dummy_sg_barrier()
  ret void
}

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
