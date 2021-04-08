; The pass operates only on DPCPP kernels - functions having "sycl_kernel" attribute
; Test to check that function calls to _Z12get_local_idj() are moved and uses replaced in an optimized
; manner if it is known that max work group size is less than 2GB.

; Check for default case i.e. max work group size < 2GB. Note that %trunc.user is removed from function,
; its uses replaced by %add. Similarly %shl.user and %ashr.inst are removed, with all uses of %ashr.inst
; replaced by %add.sext.

; RUN: opt --dpcpp-kernel-vec-clone < %s -S -o - | FileCheck %s --check-prefix=LT2GB

; LT2GB-LABEL: @_ZGVeN16uu__Z30ParallelForNDRangeImplKernel1DPiS_

; LT2GB-LABEL: entry:
; LT2GB: [[LID_CALL:%.*]] = tail call i64 @_Z12get_local_idj(i64 0)
; LT2GB-NEXT: [[LID_CALL_TRUNC:%.*]] = trunc i64 [[LID_CALL]] to i32

; LT2GB-LABEL: simd.loop:
; LT2GB-NEXT: %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.exit ]
; LT2GB-NEXT: %add = add nuw i32 [[LID_CALL_TRUNC]], %index
; LT2GB-NEXT: [[ADD_SEXT:%.*]] = sext i32 %add to i64
; LT2GB-NEXT: %non.trunc.user = add i64 [[ADD_SEXT]], 42
; LT2GB-NEXT: %other.trunc = trunc i64 %non.trunc.user to i32
; LT2GB-NEXT: %ret0 = mul i32 %other.trunc, %add
; LT2GB-NEXT: %ret1 = mul i64 %non.trunc.user, [[ADD_SEXT]]
; LT2GB-NEXT: %mul.shl = mul i64 2, [[ADD_SEXT]]
; LT2GB-NEXT: %add.shl = add i64 %mul.shl, 1
; LT2GB-NEXT: %sub.shl = add i64 %mul.shl, -1
; LT2GB-NEXT: %ret3 = mul i64 %non.trunc.user, %add.shl
; LT2GB-NEXT: %ret4 = mul i64 %non.trunc.user, %sub.shl
; LT2GB-NEXT: %shl.keep = shl i64 [[ADD_SEXT]], 32
; LT2GB-NEXT: %add2.shl = add i64 %shl.keep, 4294967296
; LT2GB-NEXT: %call = call i64 @dummy(i64 %add2.shl)
; LT2GB-NEXT: %call.ashr = ashr exact i64 %call, 32
; LT2GB-NEXT: %ret5 = mul i64 %non.trunc.user, %call.ashr
; LT2GB-NEXT: %add2 = ashr exact i64 %add2.shl, 32
; LT2GB-NEXT: %ret6 = mul i64 %non.trunc.user, %add2
; LT2GB-NEXT: br label %simd.loop.exit

; Check for non-default case i.e. max work group size > 2GB.
; RUN: opt --dpcpp-kernel-vec-clone --dpcpp-kernel-less-than-two-gig-max-work-group-size=false < %s -S -o - | FileCheck %s --check-prefix=GT2GB
; GT2GB-LABEL: @_ZGVeN16uu__Z30ParallelForNDRangeImplKernel1DPiS_

; GT2GB-LABEL: entry:
; GT2GB: [[LID_CALL:%.*]] = tail call i64 @_Z12get_local_idj(i64 0)

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
; GT2GB-NEXT: %mul.shl = mul i64 2, %shl.user
; GT2GB-NEXT: %add.shl = add i64 %mul.shl, 4294967296
; GT2GB-NEXT: %sub.shl = add i64 %mul.shl, -4294967296
; GT2GB-NEXT: %gid.add = ashr exact i64 %add.shl, 32
; GT2GB-NEXT: %gid.sub = ashr exact i64 %sub.shl, 32
; GT2GB-NEXT: %ret3 = mul i64 %non.trunc.user, %gid.add
; GT2GB-NEXT: %ret4 = mul i64 %non.trunc.user, %gid.sub
; GT2GB-NEXT: %shl.keep = shl i64 %add, 32
; GT2GB-NEXT: %add2.shl = add i64 %shl.keep, 4294967296
; GT2GB-NEXT: %call = call i64 @dummy(i64 %add2.shl)
; GT2GB-NEXT: %call.ashr = ashr exact i64 %call, 32
; GT2GB-NEXT: %ret5 = mul i64 %non.trunc.user, %call.ashr
; GT2GB-NEXT: %add2 = ashr exact i64 %add2.shl, 32
; GT2GB-NEXT: %ret6 = mul i64 %non.trunc.user, %add2
; GT2GB-NEXT: br label %simd.loop.exit

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @_Z30ParallelForNDRangeImplKernel1DPiS_(i32* nocapture readonly %in, i32* nocapture %out) #0 {
entry:
  %lid_call = tail call i64 @_Z12get_local_idj(i64 0) #2
  %non.trunc.user = add i64 %lid_call, 42
  %other.trunc = trunc i64 %non.trunc.user to i32
  %trunc.user = trunc i64 %lid_call to i32
  %shl.user = shl i64 %lid_call, 32
  %ashr.inst = ashr exact i64 %shl.user, 32
  %ret0 = mul i32 %other.trunc, %trunc.user
  %ret1 = mul i64 %non.trunc.user, %ashr.inst

  %mul.shl = mul i64 2, %shl.user ; %mul = 2 * gid
  %add.shl = add i64 %mul.shl, 4294967296  ; %gid.add = %mul + 1
  %sub.shl = add i64 %mul.shl, -4294967296 ; %gid.sub = %mul - 1
  %gid.add = ashr exact i64 %add.shl, 32
  %gid.sub = ashr exact i64 %sub.shl, 32
  %ret3 = mul i64 %non.trunc.user, %gid.add
  %ret4 = mul i64 %non.trunc.user, %gid.sub

  %shl.keep = shl i64 %lid_call, 32
  %add2.shl = add i64 %shl.keep, 4294967296
  %call = call i64 @dummy(i64 %add2.shl) ; %call blocks handling this path
  %call.ashr = ashr exact i64 %call, 32
  %ret5 = mul i64 %non.trunc.user, %call.ashr
  %add2 = ashr exact i64 %add2.shl, 32
  %ret6 = mul i64 %non.trunc.user, %add2
  ret void
}

declare i64 @dummy(i64)

;; Remaining non-vectorized function
; CHECK-LABEL: _Z30ParallelForNDRangeImplKernel1DPiS_

;; Generated vector version
; CHECK-LABEL: _ZGVeN16uu__Z30ParallelForNDRangeImplKernel1DPiS_
; CHECK-NEXT: entry:

;; Checks that we do not hit a label from the WRN region.
; CHECK-NOT: {{^[._a-zA-Z0-9]*}}:
;; get_local_id call should be hoisted outside region!
; CHECK: %call = tail call i64 @_Z12get_local_idj(i64 0)
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

declare dso_local i64 @_Z12get_local_idj(i64 %0)

attributes #0 = { "prefer-vector-width"="512" "sycl_kernel" "target-cpu"="skylake-avx512" }
