; RUN: opt -passes=sycl-kernel-barrier %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-barrier %s -S | FileCheck %s

; %j has a single user in loop exit within a barrier region.
; Check that %j's address in special buffer is loaded in for.end instead of in
; barrier region header.
;
; The IR is dumped at the beginning of BarrierPass::runOnModule() from source:
; kernel void test() {
;   int i = 170;
;   for (; i < 200;) {
;     i += 30;
;     barrier(CLK_LOCAL_MEM_FENCE);
;   }
;   j = 0;
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define dso_local void @test() #0 !no_barrier_path !2 !kernel_has_sub_groups !2 !kernel_has_global_sync !2 {
entry:
  call void @dummy_barrier.()
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 170, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %Split.Barrier.BB, %entry
  %0 = load i32, ptr %i, align 4
  %cmp = icmp slt i32 %0, 200
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32, ptr %i, align 4
  %add = add nsw i32 %1, 30
  store i32 %add, ptr %i, align 4
  br label %Split.Barrier.BB

Split.Barrier.BB:                                 ; preds = %for.body
  call void @_Z7barrierj(i32 noundef 1) #1
  br label %for.cond

for.end:                                          ; preds = %for.cond
; CHECK-LABEL: for.end:
; CHECK-NEXT: [[Index:%SBIndex[0-9]+]] = load i64, ptr %pCurrSBIndex, align 8
; CHECK-NEXT: [[Offset:%SB_LocalId_Offset[0-9]+]] = add nuw i64 [[Index]], {{[0-9]+}}
; CHECK-NEXT: [[GEP:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[Offset]]
; CHECK-NEXT: store ptr [[GEP]], ptr %j.addr
; CHECK-NEXT: [[LOAD:%[0-9]+]] = load ptr, ptr %j.addr
; CHECK-NEXT: store i32 0, ptr [[LOAD]], align 4

  store i32 0, ptr %j, align 4
  br label %Split.Barrier.BB1

Split.Barrier.BB1:                                ; preds = %for.end
  call void @_Z18work_group_barrierj(i32 1)
  ret void
}

declare void @_Z7barrierj(i32 noundef) #1

declare void @dummy_barrier.()

declare void @_Z18work_group_barrierj(i32) #2

attributes #0 = { convergent noinline norecurse nounwind optnone "kernel-call-once" "kernel-convergent-call" }
attributes #1 = { convergent nounwind "kernel-call-once" "kernel-convergent-call" }
attributes #2 = { convergent }

!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!sycl.kernels = !{!1}

!0 = !{i32 3, i32 0}
!1 = !{ptr @test}
!2 = !{i1 false}

; DEBUGIFY-COUNT-7: WARNING: Instruction with empty DebugLoc in function test
; DEBUGIFY-NOT: WARNING
