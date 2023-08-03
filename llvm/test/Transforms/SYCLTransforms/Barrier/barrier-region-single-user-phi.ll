; RUN: opt -passes=sycl-kernel-barrier %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-barrier %s -S | FileCheck %s

; %sum1.ascast.red has a single user in PHINode in a barrier region.
; This test checks that its insert point of loading from special buffer is in
; barrier region header rather than in the user basic block.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @__omp_offloading_811_294a7db__ZL5test5iPdS_S__l106(ptr %red.cpy.src.inc, i32 %a) {
entry:
  call void @dummy_barrier.()
  %sum1.ascast.red = alloca double, align 8
  %cmp0 = icmp eq i32 %a, 0
  br i1 %cmp0, label %atomic.free.red.local.update.update.exit, label %red.update.body.to.tree.preheader

; CHECK-LABEL: SyncBB1:
; CHECK-NEXT: [[Index:%SBIndex[0-9]*]] = load i64, ptr %pCurrSBIndex, align 8
; CHECK-NEXT: [[Offset:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[Index]], 0
; CHECK-NEXT: [[GEP:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[Offset]]
; CHECK-NEXT: store ptr [[GEP]], ptr %sum1.ascast.red.addr, align 8
; CHECK-NEXT: [[LOAD:%[0-9]+]] = load ptr, ptr %sum1.ascast.red.addr, align 8

red.update.body.to.tree.preheader:
  br label %red.update.body.to.tree

; CHECK-LABEL: red.update.body.to.tree:
; CHECK-NEXT: phi ptr [ %red.cpy.src.inc, %red.update.body.to.tree ], [ [[LOAD]], %red.update.body.to.tree.preheader ]

red.update.body.to.tree:
  %red.cpy.src.ptr = phi ptr [ %red.cpy.src.inc, %red.update.body.to.tree ], [ %sum1.ascast.red, %red.update.body.to.tree.preheader ]
  %cmp1 = icmp eq i32 %a, 10
  br i1 %cmp1, label %atomic.free.red.local.update.update.exit, label %red.update.body.to.tree

atomic.free.red.local.update.update.exit:
  call void @_Z18work_group_barrierj12memory_scope()
  ret void
}

declare void @_Z18work_group_barrierj12memory_scope()

declare void @dummy_barrier.()

; DEBUGIFY-COUNT-7: WARNING: Instruction with empty DebugLoc in function __omp_offloading_811_294a7db__ZL5test5iPdS_S__l106
; DEBUGIFY-NOT: WARNING
