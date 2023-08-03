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
  %a = alloca i32, align 4
  br label %sg.barrier.bb.1

; CHECK-LABEL: sg.barrier.bb.1:
; CHECK: %sg.size. = call i32 @_Z18get_sub_group_sizev()
; CHECK-NEXT: %[[#OP1:]] = zext i32 %sg.size. to i64
; CHECK-NEXT: %.splatinsert = insertelement <16 x i64> poison, i64 %[[#OP1]], i64 0
; CHECK-NEXT: %.splat = shufflevector <16 x i64> %.splatinsert, <16 x i64> poison, <16 x i32> zeroinitializer
; CHECK-NEXT: %mask.i1 = icmp ult <16 x i64> <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10,                                                                                                                                 i64 11, i64 12, i64 13, i64 14, i64 15>, %.splat
; CHECK-NEXT: %mask.i32 = sext <16 x i1> %mask.i1 to <16 x i32>
; CHECK-NEXT: %[[#OP2:]] = call <16 x i32> @_Z13sub_group_allDv16_iDv16_j(<16 x i32> zeroinitializer, <16 x i32> %mask.i32)
; CHECK-NEXT: %[[#OP3:]] = extractelement <16 x i32> %[[#OP2]], i32 0
; CHECK-NEXT: store i32 %[[#OP3]], ptr %u.call, align 4
sg.barrier.bb.1:                                  ; preds = %sg.dummy.bb.
  call void @_Z17sub_group_barrierj(i32 1)
  %call = call i32 @_Z13sub_group_alli(i32 0)
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
; DEBUGIFY: WARNING: Missing line
; DEBUGIFY-NOT: WARNING
