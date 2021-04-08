; RUN: opt --dpcpp-kernel-vec-clone < %s -S -o - | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z30ParallelForNDRangeImplKernel1DPiS_(i32* nocapture readonly %in, i32* nocapture %out) #0 {
entry:
  %call = tail call i64 @_Z12get_local_idj(i64 0)
  %arrayidx = getelementptr inbounds i32, i32* %in, i64 %call
  %0 = load i32, i32* %arrayidx, align 4
  %call1 = tail call i64 @_Z12get_local_idj(i64 0)
  %arrayidx2 = getelementptr inbounds i32, i32* %out, i64 %call1
  store i32 %0, i32* %arrayidx2, align 4
  ret void
}

; CHECK: @_Z30ParallelForNDRangeImplKernel1DPiS_{{.*}}#0
; CHECK: @_ZGVeN16uu__Z30ParallelForNDRangeImplKernel1DPiS_{{.*}}#1

; CHECK:        attributes #0 = {
; CHECK-SAME:         "dpcpp_kernel_recommended_vector_length"="16"
; CHECK-SAME:         "vectorized_kernel"="_ZGVeN16uu__Z30ParallelForNDRangeImplKernel1DPiS_"
; CHECK-SAME:         "vectorized_width"="1"

; CHECK:        attributes #1 = {
; CHECK-SAME:         "dpcpp_kernel_recommended_vector_length"="16"
; CHECK-SAME:         "scalar_kernel"="_Z30ParallelForNDRangeImplKernel1DPiS_"
; CHECK-SAME:         "vectorized_kernel"
; CHECK-SAME:         "vectorized_width"="16"

declare dso_local i64 @_Z12get_local_idj(i64 %0)

attributes #0 = { "sycl_kernel" "target-cpu"="skylake-avx512" "prefer-vector-width"="512" }
