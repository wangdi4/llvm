; RUN: opt --dpcpp-kernel-vec-clone < %s -S -o - | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z30ParallelForNDRangeImplKernel1DPiS_(i32* nocapture %out) #0 {
entry:
  %call = tail call i64 @__builtin_get_local_id(i64 0)
  %val = tail call i32 @__builtin_get_sub_group_local_id()
  %arrayidx = getelementptr inbounds i32, i32* %out, i64 %call
  store i32 %val, i32* %arrayidx, align 4
  ret void
}

; CHECK-LABEL: @_Z30ParallelForNDRangeImplKernel1DPiS_(
; CHECK-NOT: [[VAL:%.*]] = tail call i32 @__builtin_get_sub_group_local_id()
; CHECK:     store i32 0, i32* %arrayidx, align 4

; CHECK-LABEL: @_ZGVeN16u__Z30ParallelForNDRangeImplKernel1DPiS_(
; CHECK-NOT: [[VAL:%.*]] = tail call i32 @__builtin_get_sub_group_local_id()

; CHECK-LABEL: simd.loop:
; CHECK:  [[ADDSGIDX:%.*]] = add nuw i32 %index, 0
; CHECK:  store i32 [[ADDSGIDX]], i32* %arrayidx, align 4


declare dso_local i64 @__builtin_get_local_id(i64 %0)
declare dso_local i32 @__builtin_get_sub_group_local_id()

attributes #0 = { "prefer-vector-width"="512" "sycl_kernel" "target-cpu"="skylake-avx512" }

