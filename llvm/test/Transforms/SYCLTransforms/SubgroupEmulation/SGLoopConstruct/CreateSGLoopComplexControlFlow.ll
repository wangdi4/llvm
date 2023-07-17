; RUN: opt -passes='debugify,sycl-kernel-sg-emu-loop-construct,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-sg-emu-loop-construct' -S %s | FileCheck %s

; This test checks SGLoopConstruct::createSGLoop() where jump targets may exist:
; 1. inside a loop
; 2. in a conditional statement inside a loop
; 3. across a loop

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test(i32 %a) !kernel_has_sub_groups !1 !sg_emu_size !2 {
sg.loop.exclude:
; CHECK-LABEL: sg.loop.exclude:
; CHECK: [[P_LID:%sg.lid.ptr.*]] = alloca i32
; CHECK: [[P_LOOPSRC:%sg.loop.src.ptr.*]] = alloca i32
; CHECK-NOT: call i32 @_Z18get_sub_group_sizev()
  br label %entry

; CHECK-LABEL: entry:
; CHECK: store i32 0, ptr [[P_LID]]
; CHECK: store i32 [[#UID:]], ptr [[P_LOOPSRC]]
entry:                                            ; preds = %sg.loop.exclude
; CHECK: [[L_HEADER:sg.loop.header.*]]:
  call void @dummy_sg_barrier()
  %cmp = call i1 @something()
  br i1 %cmp, label %inf_loop, label %barrier_outside

inf_loop:                                         ; preds = %inf_latch, %entry
  %cmp.1 = call i1 @something()
  br i1 %cmp.1, label %break, label %inf_latch

break:                                            ; preds = %inf_loop
  %cmp.2 = call i1 @something()
  br i1 %cmp.2, label %barrier_inside, label %barrier_outside

barrier_inside:                                   ; preds = %break
; CHECK: [[L_LATCH:sg.loop.latch.*]]:
; CHECK-NEXT: [[LOOPSRC:%.*]] = load i32, ptr [[P_LOOPSRC]]
; CHECK-NEXT: switch i32 [[LOOPSRC]], label %[[L_HEADER1:sg.loop.header.*]] [
; CHECK-NEXT:   i32 [[#UID]], label %[[L_HEADER]]
; CHECK-NEXT: ]

; CHECK: [[L_HEADER2:sg.loop.header.*]]:
  call void @_Z17sub_group_barrierj(i32 1)
  br label %barrier_outside

inf_latch:                                        ; preds = %inf_loop
; CHECK: store i32 [[#UID1:]], ptr [[P_LOOPSRC]]
; CHECK: [[L_HEADER1]]:
  call void @dummy_sg_barrier()
  br label %inf_loop

barrier_outside:                                  ; preds = %break, %barrier_inside, %entry
; CHECK: [[L_LATCH1:sg.loop.latch.*]]:
; CHECK-NEXT: [[LOOPSRC1:%.*]] = load i32, ptr [[P_LOOPSRC]]
; CHECK-NEXT: switch i32 [[LOOPSRC1]], label %[[L_HEADER2]] [
; CHECK-DAG:    i32 [[#UID]], label %[[L_HEADER]]
; CHECK-DAG:    i32 [[#UID1]], label %[[L_HEADER1]]
; CHECK-NEXT: ]
  call void @_Z17sub_group_barrierj(i32 1)
  ret void
}

declare i1 @something()
declare void @_Z17sub_group_barrierj(i32)
declare void @dummy_sg_barrier()

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i1 true}
!2 = !{i32 16}

; DEBUGIFY-NOT: WARNING
