; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s | FileCheck %s

; Checks that instructions are correctly dominated in case of multiple incoming blocks sharing the same incoming value.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @kernel() !kernel_has_sub_groups !1 !sg_emu_size !2 {
; CHECK:       while.body.i.i.i:
; CHECK-NEXT:    [[SPEC_SELECT:%.*]] = select i1 false, i64 0, i64 0
; CHECK-NEXT:    [[SG_LID_1:%.*]] = call i32 @_Z22get_sub_group_local_idv()
; CHECK-NEXT:    [[TMP0:%.*]] = getelementptr <8 x i64>, ptr %w.spec.select, i32 0, i32 [[SG_LID_1]]
; CHECK-NEXT:    store i64 [[SPEC_SELECT]], ptr [[TMP0]], align 8
; CHECK-NEXT:    [[SG_LID_:%.*]] = call i32 @_Z22get_sub_group_local_idv()
; CHECK-NEXT:    [[TMP1:%.*]] = getelementptr <8 x i64>, ptr %w.spec.select, i32 0, i32 [[SG_LID_]]
; CHECK-NEXT:    [[TMP2:%.*]] = load i64, ptr [[TMP1]], align 8
; CHECK-NEXT:    br i1 true, label %while.body.i.i.i.i.1, label %if.then10.i
; CHECK:       while.body.i.i.i.i.1:
; CHECK-NEXT:    br label %if.then10.i
; CHECK:       if.then10.i:
; CHECK-NEXT:    phi i64 [ [[TMP2]], %while.body.i.i.i ]
; CHECK-SAME:            [ [[TMP2]], %while.body.i.i.i.i.1 ]
entry:
  call void @dummy_barrier.()
  br label %sg.dummy.bb.32

sg.dummy.bb.32:                                   ; preds = %entry
  call void @dummy_sg_barrier()
  br i1 false, label %land.rhs.i.i.i.preheader, label %exit

land.rhs.i.i.i.preheader:                         ; preds = %sg.dummy.bb.32
  br i1 false, label %while.body.i.i.i, label %if.then10.i

while.body.i.i.i:                                 ; preds = %land.rhs.i.i.i.preheader
  %spec.select = select i1 false, i64 0, i64 0
  br i1 true, label %while.body.i.i.i.i.1, label %if.then10.i

while.body.i.i.i.i.1:                             ; preds = %while.body.i.i.i
  br label %if.then10.i

if.then10.i:                                      ; preds = %while.body.i.i.i.i.1, %while.body.i.i.i, %land.rhs.i.i.i.preheader
  %i.0.i.i.i.lcssa24 = phi i64 [ %spec.select, %while.body.i.i.i ], [ 0, %land.rhs.i.i.i.preheader ], [ %spec.select, %while.body.i.i.i.i.1 ]
  br label %exit

exit:                                             ; preds = %if.then10.i, %sg.dummy.bb.32
  call void @dummy_sg_barrier()
  ret void
}

declare void @dummy_barrier.()

declare void @dummy_sg_barrier()

!sycl.kernels = !{!0}

!0 = !{ptr @kernel}
!1 = !{i1 true}
!2 = !{i32 8}

; DEBUGIFY-NOT: WARNING
