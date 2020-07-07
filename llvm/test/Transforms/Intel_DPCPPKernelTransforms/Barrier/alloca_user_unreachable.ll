; This test checks alloca' user is replaced with address in special buffer
; if there is unreachable in the user's basic block.
;
; RUN: opt -dpcpp-kernel-data-per-value-analysis -dpcpp-kernel-analysis -dpcpp-kernel-barrier %s -S | FileCheck %s
;

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @foo(i32 %gf) #0 {
entry:
  %gf.addr = alloca i32, align 4
  store i32 %gf, i32* %gf.addr, align 4
  ret void
}

define void @main_kernel() #1 {
entry:
; CHECK-LABEL: entry:
; CHECK: %g.addr = alloca i32*
; CHECK-NOT: %g = alloca i32

; CHECK-LABEL: SyncBB1:
; CHECK-NEXT: [[SBIndex0:%SBIndex]] = load i64, i64* %pCurrSBIndex
; CHECK-NEXT: [[Offset0:%SB_LocalId_Offset]] = add nuw i64 [[SBIndex0]], {{[0-9]+}}
; CHECK-NEXT: [[GEP0:%[0-9]+]] = getelementptr inbounds i8, i8* %pSB, i64 [[Offset0]]
; CHECK-NEXT: [[LocalId0:%pSB_LocalId]] = bitcast i8* [[GEP0]] to i32*
; CHECK-NEXT: store i32* [[LocalId0]], i32** %g.addr
; CHECK-NEXT: [[L0:%[0-9]+]] = load i32*, i32** %g.addr, align 8
; CHECK-NEXT: store i32 1, i32* [[L0]], align 4

  call void @__builtin_dpcpp_kernel_barrier_dummy()
  %g = alloca i32, align 4
  store i32 1, i32* %g, align 4
  br label %while.cond

while.cond:                                       ; preds = %entry
  br i1 false, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
; CHECK-LABEL: while.body:
; CHECK-NEXT: [[L1:%[0-9]+]] = load i32, i32* [[L0]], align 4
; CHECK-NEXT: call void @foo(i32 [[L1]]) #2

  %0 = load i32, i32* %g, align 4
  call void @foo(i32 %0) #2
  unreachable

while.end:                                        ; preds = %while.cond
  call void @__builtin_dpcpp_kernel_barrier(i32 1)
  ret void
}

declare void @__builtin_dpcpp_kernel_barrier_dummy()

; Function Attrs: convergent
declare void @__builtin_dpcpp_kernel_barrier(i32) #2

attributes #0 = { convergent noinline norecurse nounwind }
attributes #1 = { convergent noinline norecurse nounwind "dpcpp-no-barrier-path"="false" "sycl_kernel" }
attributes #2 = { convergent }

