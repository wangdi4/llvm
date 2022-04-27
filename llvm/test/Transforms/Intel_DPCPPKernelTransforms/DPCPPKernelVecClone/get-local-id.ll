; RUN: opt -dpcpp-kernel-set-vf -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes="dpcpp-kernel-set-vf,dpcpp-kernel-vec-clone" -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-set-vf -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -o - | FileCheck %s
; RUN: opt -passes="dpcpp-kernel-set-vf,dpcpp-kernel-vec-clone" -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -o - | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z30ParallelForNDRangeImplKernel1DPiS_(i32* nocapture readonly %in, i32* nocapture %out) {
entry:
  %call = tail call i64 @_Z12get_local_idj(i64 0)
  %arrayidx = getelementptr inbounds i32, i32* %in, i64 %call
  %0 = load i32, i32* %arrayidx, align 4
  %call1 = tail call i64 @_Z12get_local_idj(i64 0)
  %arrayidx2 = getelementptr inbounds i32, i32* %out, i64 %call1
  store i32 %0, i32* %arrayidx2, align 4
  ret void
}

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

; CHECK-LABEL: simd.loop.header:
; CHECK-NEXT:    [[INDEX:%.*]] = phi i32
; CHECK-NEXT:    [[LID_LINEAR:%.*]] = add nuw i32 [[TRUNC]], [[INDEX]]
; CHECK-NEXT:    [[INDEX_I64:%.*]] = sext i32 [[LID_LINEAR]] to i64
; CHECK:         [[GEP:%.*]] = getelementptr inbounds {{.*}} [[INDEX_I64]]
; CHECK:         store {{.*}} [[GEP]]
; CHECK-LABEL: simd.end.region:
; CHECK-NEXT: call void @llvm.directive.region.exit(token %entry.region)

declare dso_local i64 @_Z12get_local_idj(i64 %0)

!sycl.kernels = !{!0}
!0 = !{void (i32*, i32*)* @_Z30ParallelForNDRangeImplKernel1DPiS_}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu__Z30ParallelForNDRangeImplKernel1DPiS_ {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu__Z30ParallelForNDRangeImplKernel1DPiS_ {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu__Z30ParallelForNDRangeImplKernel1DPiS_ {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu__Z30ParallelForNDRangeImplKernel1DPiS_ {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu__Z30ParallelForNDRangeImplKernel1DPiS_ {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu__Z30ParallelForNDRangeImplKernel1DPiS_ {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu__Z30ParallelForNDRangeImplKernel1DPiS_ {{.*}} br
; DEBUGIFY-NOT: WARNING
