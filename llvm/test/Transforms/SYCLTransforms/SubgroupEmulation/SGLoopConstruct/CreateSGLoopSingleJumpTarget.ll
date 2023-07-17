; RUN: opt -passes='debugify,sycl-kernel-sg-emu-loop-construct,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-sg-emu-loop-construct' -S %s | FileCheck %s

; This test checks SGLoopConstruct::createSGLoop() where each sg_barrier only has one jump target
; 1. Alloca sg.lid.ptr and sg.loop.src.ptr in "sg.loop.exclude" BB
; 2. Initialization stores into sg.lid.ptr and sg.loop.src.ptr at each sg_dummy_barrier (loop header)
; 3. Make exiting at each sg_barrier:
;   - Increment lid
;   - lid < subgroup size ? goto latch BB : goto loop exit BB
; 4. All SG sync calls are finally removed

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
; CHECK: store i32 [[#DUMMY_ID:]], ptr [[P_LOOPSRC]]
entry:                                            ; preds = %sg.loop.exclude
; CHECK: [[L_HEADER:sg.loop.header.*]]:
; CHECK-NOT: call void @dummy_sg_barrier()
  call void @dummy_sg_barrier()
  %cmp = call i1 @something()
  br label %sg.barrier.bb.1

sg.barrier.bb.1:                                  ; preds = %entry
; CHECK: [[L_EXITING:sg.loop.exiting.*]]:
; CHECK-NEXT: [[LOOPSIZE:%.*]] = call i32 @_Z18get_sub_group_sizev()
; CHECK-NEXT: [[LID:%.*]] = load i32, ptr [[P_LID]]
; CHECK-NEXT: [[LID_INC:%.*]] = add nuw i32 [[LID]], 1
; CHECK-NEXT: store i32 [[LID_INC]], ptr [[P_LID]]
; CHECK-NEXT: [[CMP:%.*]] = icmp ult i32 [[LID_INC]], [[LOOPSIZE]]
; CHECK-NEXT: br i1 [[CMP]], label %[[L_LATCH:sg.loop.latch.*]], label %[[L_EXIT:sg.loop.exit.*]]

; JumpTargets.size() == 1
; CHECK: [[L_LATCH]]:
; CHECK-NEXT: br label %[[L_HEADER]]

; CHECK: [[L_EXIT]]:
; CHECK-NEXT: store i32 0, ptr [[P_LID]]
; CHECK-NEXT: store i32 [[#BARRIER_ID:]], ptr [[P_LOOPSRC]]
; CHECK-NEXT: br label %[[L_HEADER1:sg.loop.header.*]]

; CHECK: [[L_HEADER1]]:
; CHECK-NOT: call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z17sub_group_barrierj(i32 1)
  %cmp.1 = call i1 @something()
  br label %sg.barrier.bb.2

sg.barrier.bb.2:                                  ; preds = %sg.barrier.bb.1
; CHECK: [[L_EXITING1:sg.loop.exiting.*]]:
; CHECK-NEXT: [[LOOPSIZE1:%.*]] = call i32 @_Z18get_sub_group_sizev()
; CHECK-NEXT: [[LID1:%.*]] = load i32, ptr [[P_LID]]
; CHECK-NEXT: [[LID_INC1:%.*]] = add nuw i32 [[LID1]], 1
; CHECK-NEXT: store i32 [[LID_INC1]], ptr [[P_LID]]
; CHECK-NEXT: [[CMP1:%.*]] = icmp ult i32 [[LID_INC1]], [[LOOPSIZE1]]
; CHECK-NEXT: br i1 [[CMP1]], label %[[L_LATCH1:sg.loop.latch.*]], label %[[L_EXIT1:sg.loop.exit.*]]

; JumpTargets.size() == 1
; CHECK: [[L_LATCH1]]:
; CHECK-NEXT: br label %[[L_HEADER1]]

; CHECK: [[L_EXIT1]]:
; CHECK-NEXT: store i32 0, ptr [[P_LID]]
; CHECK-NEXT: store i32 [[#BARRIER_ID1:]], ptr [[P_LOOPSRC]]
; CHECK-NEXT: br label %[[L_HEADER2:sg.barrier.split.*]]

; CHECK: [[L_HEADER2]]:
; CHECK-NOT: call void @_Z17sub_group_barrierj(i32 1)
; CHECK: ret void
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
