; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test(ptr addrspace(1) noalias %p) !kernel_has_sub_groups !1 !sg_emu_size !2 {
entry:
  call void @dummybarrier.()
  br label %sg.dummy.bb.

sg.dummy.bb.:                                     ; preds = %entry
  call void @dummy_sg_barrier()
  %p.addr = alloca ptr addrspace(1), align 8
  %id = alloca i32, align 4
  %.compoundliteral = alloca <2 x i32>, align 8
  store ptr addrspace(1) %p, ptr %p.addr, align 8
  %call = call i32 @_Z22get_sub_group_local_idv()
  store i32 %call, ptr %id, align 4
  %0 = load i32, ptr %id, align 4
  br label %sg.barrier.bb.3

sg.barrier.bb.3:                                  ; preds = %sg.dummy.bb.
  call void @_Z17sub_group_barrierj(i32 1)
  %call1 = call i32 @_Z13sub_group_alli(i32 %0)
; CHECK: call <16 x i32> @_Z13sub_group_allDv16_iDv16_j
  br label %sg.dummy.bb.8

sg.dummy.bb.8:                                    ; preds = %sg.barrier.bb.3
  call void @dummy_sg_barrier()
  %1 = load i32, ptr %id, align 4
  br label %sg.barrier.bb.2

sg.barrier.bb.2:                                  ; preds = %sg.dummy.bb.8
  call void @_Z17sub_group_barrierj(i32 1)
  %call2 = call i32 @_Z20sub_group_reduce_addi(i32 %1)
; CHECK: call <16 x i32> @_Z20sub_group_reduce_addDv16_iDv16_j
  br label %sg.dummy.bb.7

sg.dummy.bb.7:                                    ; preds = %sg.barrier.bb.2
  call void @dummy_sg_barrier()
  %2 = load i32, ptr %id, align 4
  %vecinit = insertelement <2 x i32> undef, i32 %2, i32 0
  %3 = load i32, ptr %id, align 4
  %vecinit3 = insertelement <2 x i32> %vecinit, i32 %3, i32 1
  store <2 x i32> %vecinit3, ptr %.compoundliteral, align 8
  %4 = load <2 x i32>, ptr %.compoundliteral, align 8
  %5 = load i32, ptr %id, align 4
  br label %sg.barrier.bb.1

sg.barrier.bb.1:                                  ; preds = %sg.dummy.bb.7
  call void @_Z17sub_group_barrierj(i32 1)
  %call4 = call <2 x i32> @_Z23intel_sub_group_shuffleDv2_ij(<2 x i32> %4, i32 %5)
; CHECK: call <32 x i32> @_Z23intel_sub_group_shuffleDv32_iDv16_jS0_
  br label %sg.dummy.bb.6

sg.dummy.bb.6:                                    ; preds = %sg.barrier.bb.1
  call void @dummy_sg_barrier()
  %6 = load ptr addrspace(1), ptr %p.addr, align 8
  br label %sg.barrier.bb.

sg.barrier.bb.:                                   ; preds = %sg.dummy.bb.6
  call void @_Z17sub_group_barrierj(i32 1)
  %call5 = call i32 @_Z26intel_sub_group_block_readPU3AS1Kj(ptr addrspace(1) %6)
; CHECK: call <16 x i32> @_Z30intel_sub_group_block_read1_16PU3AS1KjDv16_j
  br label %sg.dummy.bb.5

sg.dummy.bb.5:                                    ; preds = %sg.barrier.bb.
  call void @dummy_sg_barrier()
  call void @_Z7barrierj(i32 1)
  br label %sg.dummy.bb.4

sg.dummy.bb.4:                                    ; preds = %sg.dummy.bb.5
  call void @dummy_sg_barrier()
  ret void
}

; CHECK: declare <16 x i32> @_Z13sub_group_allDv16_iDv16_j(<16 x i32>, <16 x i32>)
; CHECK: declare <16 x i32> @_Z20sub_group_reduce_addDv16_iDv16_j(<16 x i32>, <16 x i32>)
; CHECK: declare <32 x i32> @_Z23intel_sub_group_shuffleDv32_iDv16_jS0_(<32 x i32>, <16 x i32>, <16 x i32>)
; CHECK: declare <16 x i32> @_Z30intel_sub_group_block_read1_16PU3AS1KjDv16_j(ptr addrspace(1), <16 x i32>)

declare i32 @_Z22get_sub_group_local_idv()

declare i32 @_Z13sub_group_alli(i32) #0
declare i32 @_Z20sub_group_reduce_addi(i32) #1
declare <2 x i32> @_Z23intel_sub_group_shuffleDv2_ij(<2 x i32>, i32) #2
declare i32 @_Z26intel_sub_group_block_readPU3AS1Kj(ptr addrspace(1)) #3

declare void @dummybarrier.()
declare void @_Z7barrierj(i32)

declare void @_Z17sub_group_barrierj(i32)
declare void @dummy_sg_barrier()

attributes #0 = { "vector-variants"="_ZGVbM16v__Z13sub_group_alli(_Z13sub_group_allDv16_iDv16_j)" }
attributes #1 = { "vector-variants"="_ZGVbM16v__Z20sub_group_reduce_addi(_Z20sub_group_reduce_addDv16_iDv16_j)" }
attributes #2 = { "vector-variants"="_ZGVbM16vv__Z23intel_sub_group_shuffleDv2_ij(_Z23intel_sub_group_shuffleDv32_iDv16_jS0_)" }
attributes #3 = { "vector-variants"="_ZGVbM16u__Z26intel_sub_group_block_readPU3AS1Kj(_Z30intel_sub_group_block_read1_16PU3AS1KjDv16_j)" }

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i1 true}
!2 = !{i32 16}
!3 = !{!"int*"}
!4 = !{ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
; FIXME: SGValueWiden does not respect llvm.dbg.value and llvm.dbg.addr
; DEBUGIFY-COUNT-3: WARNING: Missing line
; DEBUGIFY-NOT: WARNING
