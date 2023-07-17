; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test(i32 %x) !kernel_has_sub_groups !1 !sg_emu_size !2 {
entry:
; CHECK-LABEL: wg.loop.exclude:
; CHECK-NEXT: call void @dummy_barrier.()
; CHECK-NEXT: call void @foo()
; CHECK-NEXT: call i32 @bar()
; CHECK-NEXT: br label %sg.loop.exclude
; instructions in the dummy region are moved to the WGExcludeBB
  call void @dummy_barrier.()
  call void @foo()
  %b = call i32 @bar()

; CHECK-LABEL: sg.loop.exclude:
; CHECK-NEXT: call void @dummy_barrier.()
  call void @dummy_barrier.()
  br label %sg.dummy.bb.1

sg.dummy.bb.1:                                    ; preds = %entry
  call void @dummy_sg_barrier()
  %x.addr = alloca i32, align 4
  store i32 %x, ptr %x.addr, align 4
  %0 = load i32, ptr %x.addr, align 4
  call void @dummy_barrier.()
  br label %sg.barrier.bb.

sg.barrier.bb.:                                   ; preds = %sg.dummy.bb.1
  call void @_Z17sub_group_barrierj(i32 1)
  br label %sg.dummy.bb.

sg.dummy.bb.:                                     ; preds = %sg.barrier.bb.
  call void @dummy_sg_barrier()
  call void @_Z7barrierj(i32 1)
  br label %sg.dummy.bb.2

sg.dummy.bb.2:                                    ; preds = %sg.dummy.bb.
  call void @dummy_sg_barrier()
  ret void
}

declare void @foo()
declare i32 @bar()

declare void @dummy_barrier.()
declare void @_Z7barrierj(i32)

declare void @_Z17sub_group_barrierj(i32)
declare void @dummy_sg_barrier()

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i1 true}
!2 = !{i32 16}

; DEBUGIFY-NOT: WARNING
; FIXME: SGValueWiden does not respect llvm.dbg.value and llvm.dbg.addr
; DEBUGIFY: WARNING: Missing line
; DEBUGIFY-NOT: WARNING
