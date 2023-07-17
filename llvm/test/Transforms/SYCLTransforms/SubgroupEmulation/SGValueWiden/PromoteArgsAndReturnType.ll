; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Return type is promoted from <16 x i1> to <16 x i8>
; CHECK-LABEL: define <16 x i8> @_ZGVbN16v_foo(
; Argument type is promted from <16 x i1> to <16 x i8>
; CHECK-SAME: <16 x i8> %a)

define i1 @foo(i1 %a) {
; alloca i1 is widen and promoted to alloca <16 x i8>.
; CHECK-LABEL: sg.loop.exclude:
; CHECK: %w.a.addr = alloca <16 x i8>, align 16
entry:
  call void @dummy_sg_barrier()
  %a.addr = alloca i1, align 1
; Promoted arg will be truncated to original bitwidth for each use.
; CHECK: [[SGLID_a:%.*]] = call i32 @_Z22get_sub_group_local_idv()
; CHECK: [[WIDEN_a:%.*]] = extractelement <16 x i8> %a, i32 [[SGLID_a]]
; CHECK: [[TRUNC_a:%.*]] = trunc i8 [[WIDEN_a]] to i1

; GEP on widen alloca is bitcast to original pointer type (i1*)
; CHECK: [[SGLID_addr:%.*]] = call i32 @_Z22get_sub_group_local_idv()
; CHECK: [[GEP_addr:%.*]] = getelementptr <16 x i8>, ptr %w.a.addr, i32 0, i32 [[SGLID_addr]]
; CHECK: store i1 [[TRUNC_a]], ptr [[GEP_addr]], align 1
  store i1 %a, ptr %a.addr, align 1
  %0 = load i1, ptr %a.addr, align 1
  %1 = zext i1 %0 to i32
  br label %sg.barrier.bb.

sg.barrier.bb.:                                   ; preds = %entry
  call void @_Z17sub_group_barrierj(i32 1)
  %call = call i32 @_Z13sub_group_alli(i32 %1)
  br label %sg.dummy.bb.

sg.dummy.bb.:                                     ; preds = %sg.barrier.bb.
  call void @dummy_sg_barrier()
  %call.trunc = trunc i32 %call to i1
  ret i1 %call.trunc
}

declare i32 @_Z13sub_group_alli(i32) #0

define void @test(i32 %x) !kernel_has_sub_groups !1 !sg_emu_size !2 {
; CHECK-LABEL: define void @test
entry:
  call void @dummybarrier.()
  br label %sg.dummy.bb.1

sg.dummy.bb.1:                                    ; preds = %entry
  call void @dummy_sg_barrier()
  %x.addr = alloca i32, align 4
  store i32 %x, ptr %x.addr, align 4
  %0 = load i32, ptr %x.addr, align 4
  %1 = trunc i32 %0 to i1
  br label %sg.barrier.bb.

; CHECK-LABEL: sg.barrier.bb.:
; CHECK: [[VEC_i8:%.*]] = load <16 x i8>, ptr
; CHECK: [[RET_i8:%.*]] = call <16 x i8> @_ZGVbN16v_foo(<16 x i8> [[VEC_i8]])
; CHECK: store <16 x i8> [[RET_i8]], ptr
sg.barrier.bb.:                                   ; preds = %sg.dummy.bb.1
  call void @_Z17sub_group_barrierj(i32 1)
  %call = call i1 @foo(i1 %1)
  br label %sg.dummy.bb.

; The original i1 use is proxied from promoted i8 type by trunc.
; GEP on <16 x i1>* is avoided.
; CHECK-LABEL: sg.dummy.bb.:
; CHECK: [[SGLID_call:%.*]] = call i32 @_Z22get_sub_group_local_idv()
; CHECK: [[GEP_call:%.*]] = getelementptr <16 x i8>, ptr %w.call, i32 0, i32 [[SGLID_call]]
; CHECK-NOT: getelementptr <16 x i1>, ptr
; CHECK: [[LOAD_call:%.*]] = load i8, ptr [[GEP_call]]
; CHECK: [[TRUNC_call:%.*]] = trunc i8 [[LOAD_call]] to i1
; CHECK: %call.ext = zext i1 [[TRUNC_call]] to i32
sg.dummy.bb.:                                     ; preds = %sg.barrier.bb.
  call void @dummy_sg_barrier()
  %call.ext = zext i1 %call to i32
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

attributes #0 = { "vector-variants"="_ZGVbM16v__Z13sub_group_alli(_Z13sub_group_allDv16_iDv16_j)" }

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i1 true}
!2 = !{i32 16}

; DEBUGIFY-NOT: WARNING
; FIXME: SGValueWiden does not respect llvm.dbg.value and llvm.dbg.addr
; DEBUGIFY-COUNT-1: WARNING: Missing line
; DEBUGIFY-NOT: WARNING
