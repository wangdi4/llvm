; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define i32 @foo(i32 %a) {
entry:
  call void @dummy_sg_barrier()
  %a.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  %0 = load i32, ptr %a.addr, align 4
  br label %sg.barrier.bb.

sg.barrier.bb.:                                   ; preds = %entry
  call void @_Z17sub_group_barrierj(i32 1)
  %call = call i32 @_Z13sub_group_alli(i32 %0)
  br label %sg.dummy.bb.

sg.dummy.bb.:                                     ; preds = %sg.barrier.bb.
  call void @dummy_sg_barrier()
  ret i32 %call
}

declare i32 @_Z13sub_group_alli(i32) #0

; CHECK-DAG: define i32 @foo(i32 %a)
; CHECK-DAG: define <64 x i32> @_ZGVbN64v_foo(<64 x i32> %a)
; CHECK-DAG: define <32 x i32> @_ZGVbN32v_foo(<32 x i32> %a)
; CHECK-DAG: define <16 x i32> @_ZGVbN16v_foo(<16 x i32> %a)
; CHECK-DAG: define <8 x i32> @_ZGVbN8v_foo(<8 x i32> %a)
; CHECK-DAG: define <4 x i32> @_ZGVbN4v_foo(<4 x i32> %a)

define void @test_with_emusize_64(i32 %x) !kernel_has_sub_groups !1 !sg_emu_size !2 {
entry:
  call void @dummybarrier.()
  br label %sg.dummy.bb.1

sg.dummy.bb.1:                                    ; preds = %entry
  call void @dummy_sg_barrier()
  %x.addr = alloca i32, align 4
  store i32 %x, ptr %x.addr, align 4
  %0 = load i32, ptr %x.addr, align 4
  br label %sg.barrier.bb.

sg.barrier.bb.:                                   ; preds = %sg.dummy.bb.1
  call void @_Z17sub_group_barrierj(i32 1)
  %call = call i32 @foo(i32 %0)
  br label %sg.dummy.bb.

sg.dummy.bb.:                                     ; preds = %sg.barrier.bb.
  call void @dummy_sg_barrier()
  call void @_Z7barrierj(i32 1)
  br label %sg.dummy.bb.2

sg.dummy.bb.2:                                    ; preds = %sg.dummy.bb.
  call void @dummy_sg_barrier()
  ret void
}

define void @test_with_emusize_32(i32 %x) !kernel_has_sub_groups !1 !sg_emu_size !3 {
entry:
  call void @dummybarrier.()
  br label %sg.dummy.bb.1

sg.dummy.bb.1:                                    ; preds = %entry
  call void @dummy_sg_barrier()
  %x.addr = alloca i32, align 4
  store i32 %x, ptr %x.addr, align 4
  %0 = load i32, ptr %x.addr, align 4
  br label %sg.barrier.bb.

sg.barrier.bb.:                                   ; preds = %sg.dummy.bb.1
  call void @_Z17sub_group_barrierj(i32 1)
  %call = call i32 @foo(i32 %0)
  br label %sg.dummy.bb.

sg.dummy.bb.:                                     ; preds = %sg.barrier.bb.
  call void @dummy_sg_barrier()
  call void @_Z7barrierj(i32 1)
  br label %sg.dummy.bb.2

sg.dummy.bb.2:                                    ; preds = %sg.dummy.bb.
  call void @dummy_sg_barrier()
  ret void
}

define void @test_with_emusize_16(i32 %x) !kernel_has_sub_groups !1 !sg_emu_size !4 {
entry:
  call void @dummybarrier.()
  br label %sg.dummy.bb.1

sg.dummy.bb.1:                                    ; preds = %entry
  call void @dummy_sg_barrier()
  %x.addr = alloca i32, align 4
  store i32 %x, ptr %x.addr, align 4
  %0 = load i32, ptr %x.addr, align 4
  br label %sg.barrier.bb.

sg.barrier.bb.:                                   ; preds = %sg.dummy.bb.1
  call void @_Z17sub_group_barrierj(i32 1)
  %call = call i32 @foo(i32 %0)
  br label %sg.dummy.bb.

sg.dummy.bb.:                                     ; preds = %sg.barrier.bb.
  call void @dummy_sg_barrier()
  call void @_Z7barrierj(i32 1)
  br label %sg.dummy.bb.2

sg.dummy.bb.2:                                    ; preds = %sg.dummy.bb.
  call void @dummy_sg_barrier()
  ret void
}


define void @test_with_emusize_8(i32 %x) !kernel_has_sub_groups !1 !sg_emu_size !5 {
entry:
  call void @dummybarrier.()
  br label %sg.dummy.bb.1

sg.dummy.bb.1:                                    ; preds = %entry
  call void @dummy_sg_barrier()
  %x.addr = alloca i32, align 4
  store i32 %x, ptr %x.addr, align 4
  %0 = load i32, ptr %x.addr, align 4
  br label %sg.barrier.bb.

sg.barrier.bb.:                                   ; preds = %sg.dummy.bb.1
  call void @_Z17sub_group_barrierj(i32 1)
  %call = call i32 @foo(i32 %0)
  br label %sg.dummy.bb.

sg.dummy.bb.:                                     ; preds = %sg.barrier.bb.
  call void @dummy_sg_barrier()
  call void @_Z7barrierj(i32 1)
  br label %sg.dummy.bb.2

sg.dummy.bb.2:                                    ; preds = %sg.dummy.bb.
  call void @dummy_sg_barrier()
  ret void
}

define void @test_with_emusize_4(i32 %x) !kernel_has_sub_groups !1 !sg_emu_size !6 {
entry:
  call void @dummybarrier.()
  br label %sg.dummy.bb.1

sg.dummy.bb.1:                                    ; preds = %entry
  call void @dummy_sg_barrier()
  %x.addr = alloca i32, align 4
  store i32 %x, ptr %x.addr, align 4
  %0 = load i32, ptr %x.addr, align 4
  br label %sg.barrier.bb.

sg.barrier.bb.:                                   ; preds = %sg.dummy.bb.1
  call void @_Z17sub_group_barrierj(i32 1)
  %call = call i32 @foo(i32 %0)
  br label %sg.dummy.bb.

sg.dummy.bb.:                                     ; preds = %sg.barrier.bb.
  call void @dummy_sg_barrier()
  call void @_Z7barrierj(i32 1)
  br label %sg.dummy.bb.2

sg.dummy.bb.2:                                    ; preds = %sg.dummy.bb.
  call void @dummy_sg_barrier()
  ret void
}


declare void @dummybarrier.()
declare void @_Z7barrierj(i32)

declare void @_Z17sub_group_barrierj(i32)
declare void @dummy_sg_barrier()

attributes #0 = { "vector-variants"="_ZGVbM4v__Z13sub_group_alli(_Z13sub_group_allDv4_iDv4_j),_ZGVbM8v__Z13sub_group_alli(_Z13sub_group_allDv8_iDv8_j),_ZGVbM16v__Z13sub_group_alli(_Z13sub_group_allDv16_iDv16_j),_ZGVbM32v__Z13sub_group_alli(_Z13sub_group_allDv32_iDv32_j),_ZGVbM64v__Z13sub_group_alli(_Z13sub_group_allDv64_iDv64_j)" }

!sycl.kernels = !{!0}

!0 = !{ptr @test_with_emusize_64, ptr @test_with_emusize_32, ptr @test_with_emusize_16, ptr @test_with_emusize_8, ptr @test_with_emusize_4}
!1 = !{i1 true}
!2 = !{i32 64}
!3 = !{i32 32}
!4 = !{i32 16}
!5 = !{i32 8}
!6 = !{i32 4}


; DEBUGIFY-NOT: WARNING
; FIXME: SGValueWiden does not respect llvm.dbg.value and llvm.dbg.addr
; DEBUGIFY-COUNT-5: WARNING: Missing line
; DEBUGIFY-NOT: WARNING
