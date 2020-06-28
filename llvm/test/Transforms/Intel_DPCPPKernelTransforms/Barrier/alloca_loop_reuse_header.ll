; This test checks that loop header (for.cond)'s successor (for.end) which is
; not in the loop can reuse address loaded in loop header even if the loop has
; a barrier.
;
; RUN: opt -dpcpp-kernel-analysis -dpcpp-kernel-data-per-value-analysis -dpcpp-kernel-barrier %s -S | FileCheck %s
;

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test() #0 {
entry:
; CHECK-LABEL: entry:
; CHECK: %i.addr = alloca i32*
; CHECK-NOT: %i = alloca i32
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  %i = alloca i32, align 4
  store i32 170, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %"Barrier BB1", %entry
  %0 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %0, 200
; CHECK-LABEL: for.cond:
; CHECK-NEXT: [[Index:%SBIndex[0-9]*]] = load i64, i64* %pCurrSBIndex
; CHECK-NEXT: [[Offset:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[Index]], {{[0-9]+}}
; CHECK-NEXT: [[GEP:%[0-9]+]] = getelementptr inbounds i8, i8* %pSB, i64 [[Offset]]
; CHECK-NEXT: [[LocalId:%pSB_LocalId[0-9]*]] = bitcast i8* [[GEP]] to i32*
; CHECK-NEXT: store i32* [[LocalId]], i32** %i.addr
; CHECK-NEXT: [[I:%[0-9]+]] = load i32*, i32** %i.addr
; CHECK-NEXT: load i32, i32* [[I]], align 4
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32, i32* %i, align 4
  %add = add nsw i32 %1, 30
  store i32 %add, i32* %i, align 4
  br label %"Barrier BB1"

"Barrier BB1":                                    ; preds = %for.body
  call void @__builtin_dpcpp_kernel_barrier(i32 1) #1
  br label %for.cond

for.end:                                          ; preds = %for.cond
; CHECK-LABEL: for.end:
; CHECK-NEXT: store i32 0, i32* [[I]], align 4
  store i32 0, i32* %i, align 4
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %for.end
  call void @__builtin_dpcpp_kernel_barrier(i32 1)
  ret void
}

; Function Attrs: convergent
declare void @__builtin_dpcpp_kernel_barrier(i32) #1

declare void @__builtin_dpcpp_kernel_barrier_dummy()

attributes #0 = { convergent noinline "sycl_kernel" "kernel-call-once" "kernel-convergent-call" }
attributes #1 = { convergent "kernel-call-once" "kernel-convergent-call" }

