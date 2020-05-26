; The pass operates only on DPCPP kernels - functions having "sycl_kernel" attribute
; Test to check that function calls to __builtin_get_local_id() are moved and uses replaced in an optimized
; manner if it is known that max work group size is less than 2GB.

; Check for default case i.e. max work group size < 2GB. Note that %trunc.user is removed from function,
; its uses replaced by %add. Similarly %shl.user and %ashr.inst are removed, with all uses of %ashr.inst
; replaced by %add.sext.

; RUN: opt --dpcpp-kernel-vec-clone < %s -S -o - | FileCheck %s --check-prefix=LT2GB

; LT2GB-LABEL: @_ZGVeN16uu_30ParallelForNDRangeImplKernel1DPiS_

; LT2GB-LABEL: entry:
; LT2GB: [[LID_CALL:%.*]] = tail call i64 @__builtin_get_local_id(i64 0)
; LT2GB-NEXT: [[LID_CALL_TRUNC:%.*]] = trunc i64 [[LID_CALL]] to i32

; LT2GB-LABEL: simd.loop:
; LT2GB-NEXT: %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.exit ]
; LT2GB-NEXT: %add = add nuw i32 [[LID_CALL_TRUNC]], %index
; LT2GB-NEXT: [[ADD_SEXT:%.*]] = sext i32 %add to i64
; LT2GB-NEXT: %non.trunc.user = add i64 [[ADD_SEXT]], 42
; LT2GB-NEXT: %other.trunc = trunc i64 %non.trunc.user to i32
; LT2GB-NEXT: %ret0 = mul i32 %other.trunc, %add
; LT2GB-NEXT: %ret1 = mul i64 %non.trunc.user, [[ADD_SEXT]]
; LT2GB-NEXT: br label %simd.loop.exit

; Check for non-default case i.e. max work group size > 2GB.
; RUN: opt --dpcpp-kernel-vec-clone --dpcpp-kernel-less-than-two-gig-max-work-group-size=false < %s -S -o - | FileCheck %s --check-prefix=GT2GB
; GT2GB-LABEL: @_ZGVeN16uu_30ParallelForNDRangeImplKernel1DPiS_

; GT2GB-LABEL: entry:
; GT2GB: [[LID_CALL:%.*]] = tail call i64 @__builtin_get_local_id(i64 0)

; GT2GB-LABEL: simd.loop:
; GT2GB-NEXT: %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.exit ]
; GT2GB-NEXT: [[IDX_SEXT:%.*]] = sext i32 %index to i64
; GT2GB-NEXT: %add = add nuw i64 [[IDX_SEXT]], %lid_call
; GT2GB-NEXT: %non.trunc.user = add i64 %add, 42
; GT2GB-NEXT: %other.trunc = trunc i64 %non.trunc.user to i32
; GT2GB-NEXT: %trunc.user = trunc i64 %add to i32
; GT2GB-NEXT: %shl.user = shl i64 %add, 32
; GT2GB-NEXT: %ashr.inst = ashr exact i64 %shl.user, 32
; GT2GB-NEXT: %ret0 = mul i32 %other.trunc, %trunc.user
; GT2GB-NEXT: %ret1 = mul i64 %non.trunc.user, %ashr.inst
; GT2GB-NEXT: br label %simd.loop.exit

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @_Z30ParallelForNDRangeImplKernel1DPiS_(i32* nocapture readonly %in, i32* nocapture %out) #0 {
entry:
  %lid_call = tail call i64 @__builtin_get_local_id(i64 0) #2
  %non.trunc.user = add i64 %lid_call, 42
  %other.trunc = trunc i64 %non.trunc.user to i32
  %trunc.user = trunc i64 %lid_call to i32
  %shl.user = shl i64 %lid_call, 32
  %ashr.inst = ashr exact i64 %shl.user, 32
  %ret0 = mul i32 %other.trunc, %trunc.user
  %ret1 = mul i64 %non.trunc.user, %ashr.inst
  ret void
}

;; Remaining non-vectorized function
; CHECK-LABEL: _Z30ParallelForNDRangeImplKernel1DPiS_

;; Generated vector version
; CHECK-LABEL: _ZGVeN16uu_30ParallelForNDRangeImplKernel1DPiS_
; CHECK-NEXT: entry:

;; Checks that we do not hit a label from the WRN region.
; CHECK-NOT: {{^[._a-zA-Z0-9]*}}:
;; get_local_id call should be hoisted outside region!
; CHECK: %call = tail call i64 @__builtin_get_local_id(i64 0)
; CHECK: [[TRUNC:%.*]] = trunc i64 %call to i32
; CHECK: label %simd.begin.region

; CHECK-LABEL: simd.begin.region:
; CHECK-NEXT:    %entry.region = call token @llvm.directive.region.entry()
; CHECK-NEXT:    br label %simd.loop.preheader

; CHECK-LABEL: simd.loop:
; CHECK-NEXT:    [[INDEX:%.*]] = phi i32
; CHECK-NEXT:    [[LID_LINEAR:%.*]] = add nuw i32 [[TRUNC]], [[INDEX]]
; CHECK-NEXT:    [[INDEX_I64:%.*]] = sext i32 [[LID_LINEAR]] to i64
; CHECK:         [[GEP:%.*]] = getelementptr inbounds {{.*}} [[INDEX_I64]]
; CHECK:         store {{.*}} [[GEP]]
; CHECK-LABEL: simd.end.region:
; CHECK-NEXT: call void @llvm.directive.region.exit(token %entry.region)

declare dso_local i64 @__builtin_get_local_id(i64 %0)

attributes #0 = { "prefer-vector-width"="512" "sycl_kernel" "target-cpu"="skylake-avx512" }
