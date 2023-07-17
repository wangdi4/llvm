; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define <16 x i32> @foo(i32 %a) {
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

  %vec = insertelement <16 x i32> undef, i32 %call, i32 0
  %vec.ret = shufflevector <16 x i32> %vec, <16 x i32> undef, <16 x i32> zeroinitializer

  ret <16 x i32> %vec.ret
}

declare i32 @_Z13sub_group_alli(i32) #0

define void @test(i32 %x) !kernel_has_sub_groups !1 !sg_emu_size !2 {
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
  %call.vec = call <16 x i32> @foo(i32 %0)
  br label %sg.dummy.bb.

sg.dummy.bb.:                                     ; preds = %sg.barrier.bb.
  call void @dummy_sg_barrier()
  %call = extractelement <16 x i32> %call.vec, i32 0

  call void @_Z7barrierj(i32 1)
  br label %sg.dummy.bb.2

sg.dummy.bb.2:                                    ; preds = %sg.dummy.bb.
  call void @dummy_sg_barrier()
  ret void
}


; CHECK: define <256 x i32> @_ZGVbN16v_foo(<16 x i32> %a)
; CHECK: sg.loop.exclude:
; CHECK: %w.ret = alloca <256 x i32>, align 1024

; CHECK: %sg.lid.1 = call i32 @_Z22get_sub_group_local_idv()
; CHECK-NEXT: [[MUL:%.*]] = mul i32 %sg.lid.1, 16
; CHECK-NEXT: [[PTR:%.*]] = getelementptr <256 x i32>, ptr %w.ret, i32 0, i32 [[MUL]]
; CHECK-NEXT: store <16 x i32> %vec.ret, ptr [[PTR]], align 64

; CHECK: sg.dummy.bb.:
; CHECK: call void @_Z17sub_group_barrierj(i32 1)
; CHECK-NEXT: [[RET:%.*]] = load <256 x i32>, ptr %w.ret, align 1024
; CHECK-NEXT: ret <256 x i32> [[RET]]

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
; DEBUGIFY-COUNT-1: WARNING: Missing line
; DEBUGIFY-NOT: WARNING
