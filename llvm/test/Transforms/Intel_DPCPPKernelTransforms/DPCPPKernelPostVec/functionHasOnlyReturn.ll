; Functions with only return instructions are not eligible for VecClone and VPlan.
; RUN: opt -dpcpp-kernel-postvec %s -S

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @_Z30ParallelForNDRangeImplKernel1DPiS_(i32* nocapture readonly %in, i32* nocapture %out) #0 {
entry:
  ret void
}

declare dso_local i64 @_Z12get_local_idj(i64 %0) local_unnamed_addr

attributes #0 = { "sycl_kernel" }
